package render_callback;

use 5.006;
use warnings;
use strict;
use base qw(CAM::PDF::GS);

use Dumpvalue;

# Written in 2011 by Timothy B. Lee, tblee@princeton.edu

# To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
# rights to this software to the public domain worldwide. This software is distributed without any warranty.

# You should have received a copy of the CC0 Public Domain Dedication along with this software. If not,
# see <http://creativecommons.org/publicdomain/zero/1.0/>.

# THIS FILE APPEARS TO BE DEPRECATED.

# CAM::PDF has a callback mechanism where it walks the PDF tree and calls a specified function
# (renderText in this case) on each node.

our $VERSION = '1.52';

my @list;

my $pagenum;

sub init
{
  $pagenum = shift;

  @list = ();
}

sub renderText
{
  my $self = shift;
  my $string = shift;
  my $width = shift;

  my $state = pdf_state->new_from_node($self, $pagenum);

  $state->process_text($string,0,$width);

  push @list, $state->{objects}->[0];
}

sub getList
{
  return @list;
}

1;

