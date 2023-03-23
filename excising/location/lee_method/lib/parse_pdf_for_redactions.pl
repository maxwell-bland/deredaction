#!/usr/bin/perl -w

# Written in 2011 by Timothy B. Lee, tblee@princeton.edu

# To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
# rights to this software to the public domain worldwide. This software is distributed without any warranty.

# You should have received a copy of the CC0 Public Domain Dedication along with this software. If not,
# see <http://creativecommons.org/publicdomain/zero/1.0/>.

# This is my big rectangle-finding library. It requires the CAM::PDF library.

my $debug = 0;

use 5.006;
use warnings;
use strict;
use Data::Dumper;
require "lib/pdf_state.pm";
require "lib/pdf_object.pm";
require "lib/extract_text_objects.pl";

use List::Util qw[min max];
use CAM::PDF;
use CAM::PDF::Content;

require "lib/find_xxed_files.pl";

# Pretty printing for debugging.

require 'dumpvar.pl';

sub get_bad_redactions
{
  my ($infile) = @_;

  my $pdf = CAM::PDF->new($infile) || return ();

  my $pages = $pdf->numPages();

  my %redactions = ();

  for(my $pagenum = 1; $pagenum <=$pages; $pagenum++)
  {
    my $redacted_text = '';
    my ($rects_ref, $texts_ref) = get_objects_for_page($pdf, $pagenum);

    my @rects = @$rects_ref;

    my %pages = ();

    my $stupid_nysd_flag = 0;

    my @redaction_rects = ();

    @rects = remove_stupid_nysd_rects($infile, @rects);

    foreach my $rect (@rects)
    {
      if(is_redaction($rect))
      {
        foreach my $text (@$texts_ref)
        {
          if(is_bad_redaction($rect, $text))
          {
            if($debug>1)
            {
              print "Bad redaction!\n";
              print "Rect is ".$rect->to_string."\n";
              print "Text is ".$text->to_string."\n";
            }

            $redacted_text .= $text->{text};
          }
        }
        if($redacted_text ne '')
        {
          $redactions{$pagenum} .= $rect->to_string."\n";
        }
      }
    }
  }

  return \%redactions;
}

# return 1 if the given rectangle overlaps "enough" with the given
# text object.

sub is_bad_redaction
{
  return 1;
  my $rect = shift;
  my $text = shift;

  my $text_middle = $text->{y} + $text->{height}/2;
  my $text_left   = $text->{x};
  my $text_right  = $text->{x} + $text->{width};

  # This is a huge hack to deal with the fact that we don't care about
  # "redacting" whitespace. It's too much work to compute the exact width of
  # whitespace, so we approximate and say that the average space character is
  # 0.2 text units and the average font is 10 point, giving us an average
  # width of 2.0. We don't care *too* much about edge cases because slight
  # overlaps aren't the kind of thing we're most worried about.

  if($text->{text} =~ /(\s+)\S/)
  {
    $text_left += 2;
  }

  if($text->{text} =~ /\S(\s+)$/)
  {
    $text_right -= 2;
  }

  my $rect_bottom = $rect->{y};
  my $rect_top    = $rect->{y} + $rect->{height};
  my $rect_left   = $rect->{x};
  my $rect_right  = $rect->{x} + $rect->{width};

  return 0 if($text->{text} =~ /^\s+$/);

  return 0 if($rect_bottom > $text_middle);
  return 0 if($rect_top    < $text_middle);
  return 0 if($rect_left   > $text_right-2.5);
  return 0 if($rect_right  < $text_left+2.5);
  return 1;
}

# Returns a hash of tag lists representing the kinds of redaction on each page.
#
# Note that the
# method used to find pages is extremely clunky, so don't rely on
# it for anything important.

sub get_redactions_by_page
{
  my ($infile) = @_;

  if($debug>1)
  {
    print $infile."\n";
  }

  my ($rects_ref, $texts_ref) = get_objects($infile);

  return () if(!defined($rects_ref));

  my @rects = @$rects_ref;

  my %pages = ();

  my $stupid_nysd_flag = 0;

  @rects = remove_stupid_nysd_rects($infile, @rects);

  foreach(@rects)
  {
    my %rect = %$_;

    my $fc = $rect{'fill_color'};
    my $sc = $rect{'stroke_color'};
    my $mediabox_ref = $rect{'mediabox'};

    if(is_redaction($_))
    {
      if($debug)
      {
        print "YES for rect on page ".$rect{'page'}.": \n   ".
            rect_to_string(\%rect).
            " Fill: ".$fc->to_string.
            "\n   Stroke: ".$sc->to_string.
            " Media box: ".rect_to_string($mediabox_ref).
            " Op name: ".$rect{'draw_op'}."\n";

      }

      my $page = $rect{'page'};

      $pages{$page} = add_tag_to_list($pages{$page}, 'rect');
    }
    else
    {
      if($debug>1)
      {
        print "NO for rect on page ".$rect{'page'}.": \n   ".
            rect_to_string(\%rect).
            " Fill: ".$fc->to_string.
            "\n   Stroke: ".$sc->to_string.
            " Media box: ".rect_to_string($mediabox_ref).
            " Op name: ".$rect{'draw_op'}."\n";

      }
    }
  }

  my $pagenum = 0;

  foreach(@$rects_ref)
  {
    print $_->to_string."\n";
  }

  foreach(@$texts_ref)
  {
    print $_->to_string."\n";
  }

  #dumpValue($texts_ref);
  #dumpValue($rects_ref);

  my @texts = get_text_list_from_hash_list($texts_ref);

  foreach(@texts)
  {
    $pagenum++;

    if(defined($_) && /xx/i)
    {
      my $tag = get_tag_for_line($_);

      $pages{$pagenum} = add_tag_to_list($pages{$pagenum}, 'xx');

      $pages{$pagenum} = add_tag_to_list($pages{$pagenum}, $tag);

      if(is_yes_tag($tag) && $tag ne 'unknown')
      {
        $pages{$pagenum} = add_tag_to_list($pages{$pagenum}, 'yes');
      }
    }
  }

  return %pages;
}

sub add_tag_to_list
{
  my($tags, $tag) = @_;

  if(!$tags)
  {
    return $tag;
  }

  my @tags = split(",", $tags);

  foreach(@tags)
  {
    return $tags if($_ eq $tag);
  }

  return $tags.",".$tag;
}

# Returns an array listing all the redacted pages. Note that the
# method used to find pages is extremely clunky, so don't rely on
# it for anything important.

sub get_redacted_pages
{
  my ($infile) = @_;

  if($debug>1)
  {
    print $infile."\n";
  }

  my ($rects_ref, $texts_ref) = get_objects($infile);

  return () if(!defined($rects_ref));

  my @rects = @$rects_ref;

  my %pages = ();

  my $stupid_nysd_flag = 0;

  @rects = remove_stupid_nysd_rects($infile, @rects);

  foreach(@rects)
  {
    my %rect = %$_;

    my $fc = $rect{'fill_color'};
    my $sc = $rect{'stroke_color'};
    my $mediabox_ref = $rect{'mediabox'};

    if(is_redaction($_))
    {
      if($debug)
      {
        print "YES for rect on page ".$rect{'page'}.": \n   ".
            rect_to_string(\%rect).
            " Fill: ".$fc->to_string.
            "\n   Stroke: ".$sc->to_string.
            " Media box: ".rect_to_string($mediabox_ref).
            " Op name: ".$rect{'draw_op'}."\n";

      }

      $pages{$rect{'page'}} = 1;
    }
    else
    {
      if($debug>1)
      {
        print "NO for rect on page ".$rect{'page'}.": \n   ".
            rect_to_string(\%rect).
            " Fill: ".$fc->to_string.
            "\n   Stroke: ".$sc->to_string.
            " Media box: ".rect_to_string($mediabox_ref).
            " Op name: ".$rect{'draw_op'}."\n";

      }
    }
  }

  my @texts = get_text_list_from_hash_list($texts_ref);

  my $pagenum = 0;

  foreach(@texts)
  {
    $pagenum++;

    if(defined($_) && /xx/i)
    {
      my $tag = get_tag_for_line($_);
      if(is_yes_tag($tag))
      {
        $pages{$pagenum} = 1;
      }
    }
  }

  return sort {$a <=> $b} (keys(%pages));
}


sub get_text_list_from_hash_list
{
  my $text_hashes = shift;

  my %texts_by_page = ();

  my $max_page = -1;

  foreach(@$text_hashes)
  {
    my %text_hash = %$_;

    #dumpValue($_);

    if($text_hash{'page'} > $max_page)
    {
      $max_page = $text_hash{'page'};
    }

    if(exists($texts_by_page{$text_hash{'page'}}))
    {
      $texts_by_page{$text_hash{'page'}} .= " ".$text_hash{'text'};
    }
    else
    {
      $texts_by_page{$text_hash{'page'}} = $text_hash{'text'};
    }
  }

  my @texts = ();

  for(my $pagenum=1; $pagenum < $max_page; $pagenum++)
#  for(my $pagenum=8; $pagenum < 9; $pagenum++)
  {
    if(exists($texts_by_page{$pagenum}))
    {
      push @texts, $texts_by_page{$pagenum};
    }
    else
    {
      push @texts, '';
    }
  }

  return @texts;
}

# Takes a rectangle and returns 1 if it's a redaction rectangle.

sub is_redaction
{
  my $rect_ref = shift;
  my %rect = %$rect_ref;

  my $mediabox_ref = $rect{'mediabox'};

  my $fc = $rect{'fill_color'};
  my $sc = $rect{'stroke_color'};

  return 0 if($rect{'height'}<8);

  return 0 if($rect{'height'}>32);

  return 0 if(!rect_contains($mediabox_ref, \%rect));

  return 0 if($rect{'width'}<$rect{'height'}*1.5);

  my $fill_grey_value = $fc->{'grey'};
  my $stroke_grey_value = $sc->{'grey'};

  if(defined($fill_grey_value))
  {
    return 0 if(($rect{'draw_op'} eq 'f' || $rect{'draw_op'} eq 'f*')
                && $fill_grey_value > 0.1);
    if(defined($stroke_grey_value))
    {
      return 0 if(($rect{'draw_op'} eq 'B' || $rect{'draw_op'} eq 'f*')
                && $fill_grey_value > 0.1 
                && $stroke_grey_value == $fill_grey_value);
    }
  }

  return 1;
}

# Certain NYSD docs have a black rectangle covered by a grey rectangle. This is
# an incredibly hacky way to exclude those rectangles from my results.

sub remove_stupid_nysd_rects
{
  my $infile = shift;

  my $stupid_grey_rect_flag = 0;

  my %rect;
  my $c;

  foreach(@_)
  {
    %rect = %$_;
    $c = $rect{'fill_color'};

    if($rect{'x'} == 121.5 && $rect{'y'} == 72.75 && 
       $rect{'width'} == 354.75 && $rect{'height'} == 12.75 && 
       $c->{'red'} == 0.902 && $c->{'blue'} == 0.902 && 
       $c->{'green'} == 0.902)
    {
      if($debug>1 && !$stupid_grey_rect_flag)
      {
        print "Flagging in $infile...\n";
        print "  x is ".$rect{'x'}." and y is ".$rect{'y'}."\n";
        print "  width is ".$rect{'width'}." and height is ".$rect{'height'}."\n";
        print "  Color red is ".$c->{'red'}.", green is ".$c->{'green'}.
            " and blue is ".$c->{'blue'}."\n\n";
      }

      $stupid_grey_rect_flag = 1;
    }
  }

  return @_ if(!$stupid_grey_rect_flag);

  my @new_list = ();

  foreach(@_)
  {
    %rect = %$_;
    $c = $rect{'fill_color'};

    if(!($rect{'x'} == 121.5 && $rect{'y'} == 57 && 
       $rect{'width'} == 354.75 && $rect{'height'} == 15.75 && 
       $c->{'red'} == 0 && $c->{'blue'} == 0 && 
       $c->{'green'} == 0))
    {
      push @new_list, $_;
    }
    else
    {
      if($debug>1)
      {
        print "Discarding in $infile...\n";
        print "  x is ".$rect{'x'}." and y is ".$rect{'y'}."\n";
        print "  width is ".$rect{'width'}." and height is ".$rect{'height'}."\n";
        print "  Color red is ".$c->{'red'}.", green is ".$c->{'green'}.
              " and blue is ".$c->{'blue'}."\n\n";
      }
    }
  }

  return @new_list;
}

# Takes two rects with fields x, y, width, and height and returns 1 if
# the first one contains the second one. We use this to exclude rectangles
# that extend off the edge of the page, which are unlikely to be redaction
# rects.

sub rect_contains
{
  my $r1 = shift;
  my $r2 = shift;

  return 0 if($r1->{x} > $r2->{x});
  return 0 if($r1->{x}+$r1->{width} < $r2->{x} + $r2->{width});
  return 0 if($r1->{y} > $r2->{y});
  return 0 if($r1->{y}+$r1->{height} < $r2->{y} + $r2->{height});

  return 1;
}

sub get_objects_for_page
{
  my $pdf = shift;
  my $pagenum = shift;
  my @rects;
  my @texts;
  my %mediabox;
  my $error;

  my $rects_ref;
  my $texts_ref;

  my $page = $pdf->getPage($pagenum);

  if(defined $page->{MediaBox})
  {
    my $box = $pdf->getValue($page->{MediaBox});

    $mediabox{'x'}      = $pdf->getValue($box->[0]);
    $mediabox{'y'}      = $pdf->getValue($box->[1]);
    $mediabox{'width'}  = $pdf->getValue($box->[2]);
    $mediabox{'height'} = $pdf->getValue($box->[3]);
  }
  else
  {
    $mediabox{'x'}      = 0;
    $mediabox{'y'}      = 0;
    $mediabox{'width'}  = 612;
    $mediabox{'height'} = 792;
  }

  # First we look for black rectangles in the main page content. In
  # practice these won't exist very often.

  my @properties;

  eval
  {
    my $pagetree = $pdf->getPageContentTree($pagenum);

    ($rects_ref, $texts_ref) = parse_pagetree($pagetree, $pagenum, \%mediabox);

    push @rects, @$rects_ref;

    # We discard texts_ref because the CAM::PDF rendering functionality works
    # better than parse_pagetree does.

    push @texts, get_text_rects($pagetree, $pagenum);

    @properties = $pdf->getPropertyNames($pagenum);
  };

  if ( $error = $@ && $debug>0)
  {
    warn "Caught error: $error";
  }

  # Black rectangles are often to be found in "XObject" resources.
  # We loop through those.

  foreach(@properties)
  {
    # CAM::PDF crashes a lot and fixing it properly would be too much work,
    # so we just wrap it in an eval and ignore any errors. Worst-case
    # scenario we just miss some stuff.

    eval
    { 
      my $property = $pdf->getProperty($pagenum, $_);

      # Here return indicates exiting from the eval.
      return if(!defined($property));

      my $propval = $pdf->getValue($property);

      my $type = $propval->{Type}->{value};

      # Here return indicates exiting from the eval.

      return if(!defined($type) || $type ne 'XObject');

      # This is a horrible hack to deal with the limitations of CAM::PDF.
      # decodeOne() expects a hash with key "type" and value "dictionary,"
      # so that's what we're going to give it.

      my %dictionary = ('type' => 'dictionary', 'value' => $propval);

      # The XObject will have a stream of PDF commands just like the main
      # page content. We extract it and then pass it to our same function.

      my $content = $pdf->decodeOne(\%dictionary);
      my $tree = CAM::PDF::Content->new($content);
      ($rects_ref, $texts_ref) = 
              parse_pagetree($tree, $pagenum, \%mediabox);

      $texts_ref = get_text_rects($tree, $pagenum);

      push @rects, @$rects_ref;
      push @texts, @$texts_ref;
    };

    if ( $error = $@ && $debug>0)
    {
      warn "Caught error: $error";
    }
  }

  return (\@rects, \@texts);
}

# Returns a list of relevant files.

sub get_objects
{
  my($infile) = @_;

  my $pdf = CAM::PDF->new($infile) || return ();

  my $pages = $pdf->numPages();

  my @rects;
  my @texts;

  my $rects_ref;
  my $texts_ref;

  my $error;

  for(my $pagenum=1; $pagenum<=$pages; $pagenum++)
  {
    ($rects_ref, $texts_ref) = get_objects_for_page($pdf, $pagenum);
    push(@rects, @$rects_ref);
    push(@texts, @$texts_ref);
  }

  return (\@rects, \@texts);
}

sub rect_to_string
{
  my $r = shift;

  return "(".$r->{x}.", ".$r->{y}.") ".$r->{width}." by ".$r->{height};
}

# Find all the rectangles and "XX"-style redactions in a page tree

sub parse_pagetree
{
  my @rects = ();
  my @texts = ();
  my $pagetree = shift;
  my $pagenum = shift;
  my $mediabox_ref = shift;

  my $state = pdf_state->new($pagenum, $mediabox_ref);

  return (\@rects, \@texts) if(!defined($pagetree));

  my @stack = ([@{$pagetree->{blocks}}]);

  # The stack is a list of blocks.  We do depth-first on blocks, but
  # we must be sure to traverse the children of the blocks in their
  # original order.

  my $level = 0;

  while (@stack > 0)
  {
    # keep grabbing the same node until it's empty
    my $node = $stack[-1];

    if (ref $node)
    {
      if (@{$node} > 0)   # Still has children?
      {
	my $block = shift @{$node};   # grab the next child

	if ($block->{type} eq 'block')
	{
	  if ($block->{name} eq 'BT')
	  {
	    # Insert a flag on the stack to say when we leave the BT block
	    push @stack, 'BT';
            $state->start_text_block;
	  }
	  push @stack, [@{$block->{value}}];  # descend
	}
	else
	{
	  if ($block->{type} ne 'op')
	  {
	    die 'misconception';
	  }

          # print "Processing...\n\n";

          # dumpValue($block);

          $state->process($block);
	}
      }
      else
      {
	# Node is now empty, clear it from the stack
	pop @stack;
      }
    }
    else
    {
      # This is the 'BT' flag we pushed on the stack above
      pop @stack;
    }
  }

  return $state->get_rects, $state->get_texts;
}

#sub color_to_string
#{
#  my $c = shift;
#
#  return "Unknown color" if(!defined($c));
#  return "Unknown color" if(!exists($c->{type}));
#
#  my $s = " (G:".sprintf("%0.3f",$c->{grey}).")";
#
#  if($c->{type} eq 'cs')
#  {
#    return $c->{type}."(".$c->{name}.")".$s;
#  }
#  elsif($c->{type} eq 'sc')
#  {
#    return $c->{type}."(".$c->{args}.") in ".$c->{cs}.$s;
#  }
#  elsif($c->{type} eq 'rgb')
#  {
#    return $c->{type}."(".$c->{red}.",".$c->{green}.",".$c->{blue}.")".$s;
#  }
#  elsif($c->{type} eq 'scn')
#  {
#    return $c->{type}."(".$c->{args}.") in ".$c->{cs}.$s;
#  }
#  elsif($c->{type} eq 'grey')
#  {
#    return $c->{type}."(".$c->{grey}.")".$s;
#  }
#  elsif($c->{type} eq 'cymk')
#  {
#    return $c->{type}."(".$c->{cyan}.",".$c->{yellow}.
#           ",".$c->{magenta}.",".$c->{black}.")".$s;
#  }
#  else
#  {
#    return "Unknown color";
#  }
#}

return 1;
