#!/usr/bin/perl

# Written in 2011 by Timothy B. Lee, tblee@princeton.edu

# To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
# rights to this software to the public domain worldwide. This software is distributed without any warranty.

# You should have received a copy of the CC0 Public Domain Dedication along with this software. If not,
# see <http://creativecommons.org/publicdomain/zero/1.0/>.


# Definitions...
# SSN: Social Security number
# account: bank account number
# treasury: treasury bank account number (May be a false positive)
# id: Other ID number
# text: Redacted text (human determined)
# address: Mailing address
# email: Email address
# name: Name of adult person
# minor: Name of minor person
# ts: Trade secret
# jury: Information about the identity of jury members
# ns: national security issues
# admin: Administrative law
# yes: Generic accept

@yes_tags = ('unknown','ssn','account','treasury','address','bosco',
             'email','name','minor','text','yes', 'ts', 'malamud', 'jury',
             'privilege', 'phone', 'health', 'ns', 'admin', 'id', 'ip',
             'signature', 'amount');

sub get_yes_tags
{
  return @yes_tags;
}

# Definitions...
# double: "XX"
# triple: "XXX"
# vioxx: "Vioxx"
# exxon: "Exxon"
# cmark: X's used as a checkmark: XX or XXX
# roman: Roman numeral ("XXVI", "XXXI", etc)
# word: Mix of x's and other letters
# strike: X's used to cross out (but not actually remove) characters
# form: X's used to indicate blanks in a form letter
# serial: Model number like "8xx" or "NX50XX"
# porn: "XXX" as pornographic signifier
# no: Generic reject

@no_tags = ("double","triple","vioxx","exxon","cmark","roman",
            "word",'strike','form',"serial",'porn',"highlight",'no',
            "birth_dates_that_state");

sub get_no_tags
{
  return @no_tags;
}

@noskip_tags = ("unknown", "numeric", "date", "rect");
@skip_tags = ("xx");

@ok_tags = (@noskip_tags, @skip_tags, "bad");

sub get_ok_tags
{
  return @ok_tags;
}

# This function doesn't appear to actually get used. And good
# thing because it references a totally random filehandle G.

sub string_is_xx_redaction
{
  my $s = shift;

  foreach(<G>)
  {
    if(/xx/i)
    {
      $flag = 1;

      $tag = get_tag_for_line($_);

      # If we find a redacted one, break out of the loop
      if(is_yes_tag($tag))
      {
        return 1;
      }
    }
  }
}

# Given a line of text, returns the appropriate tag. One flaw that ought to be
# fixed in a future version is that it returns a single tag regardless of the
# length of the line. Since a "line" will in some circumstances will be
# an entire page, it should really return a list of tags rather than 
# a single tag.

sub get_tag_for_line
{
  ($line) = @_;

  if($line =~ /Birth dates that state "1\/1\/XX" indicate unknown month and day of birth./)
  {
    return "birth_dates_that_state";
  }

  if($line =~ /[0-9x]{3}-[0-9x]{2}-[0-9x]{3}/i)
  {
    return "ssn";
  }

  # This is a hack to identify some random documents in Alaska that
  # show up repeatedly. A better approach can probably be found.

  if($line =~ /0022-6855XX/i)
  {
    return "treasury";
  }

  if(
      ($line =~ /^(.*\s)?[x\d\-]*xx[x\d\-]\d[x\d\-]*(\s.*)$/i) ||
      ($line =~ /^(.*\s)?[x\d\-]*\d[\dx\-]xx[x\d\-]*(\s.*)$/i) ||
      ($line =~ /^(.*\s)?[x\d\-]*xx[x\d\-]\-[x\d\-]*(\s.*)$/i) ||
      ($line =~ /^(.*\s)?[x\d\-]*\-[\dx\-]xx[x\d\-]*(\s.*)$/i)
    )
  {
    return "numeric";
  }

  if(
      ($line =~ /x[\dx]+\/[\dx]+\/[\dx]+/i) ||
      ($line =~ /[\dx]+\/x[\dx]+\/[\dx]+/i) ||
      ($line =~ /[\dx]+\/[\dx]+\/x[\dx]+/i) 
    )
  {
    return "date";
  }

  if($line =~ /vioxx/i)
  {
    return "vioxx";
  }

  if($line =~ /exxon/i)
  {
    return "exxon";
  }

  if(($line =~ /\(\s*xxx?\s*\)/i) ||
     ($line =~ /\[\s*xxx?\s*\]/i) ||
     ($line =~ /_xxx?_/i))
  {
    return "checkmark";
  }

  if($line =~ /[a-wyz]xx/i || $line =~/xx[a-wyz]/i)
  {
    return "word";
  }

  return "unknown";
}

sub is_yes_tag
{
  ($tag) = @_;

  foreach(@yes_tags)
  {
    return 1 if($tag eq $_);
  }
  return 0;
}

sub is_no_tag
{
  ($tag) = @_;

  foreach(@no_tags)
  {
    return 1 if($tag eq $_);
  }
  return 0;
}

sub is_ok_tag
{
  ($tag) = @_;

  foreach(@ok_tags)
  {
    return 1 if($tag eq $_);
  }
  return 0;
}

sub is_noskip_tag
{
  ($tag) = @_;

  foreach(@noskip_tags)
  {
    return 1 if($tag eq $_);
  }
  return 0;
}


