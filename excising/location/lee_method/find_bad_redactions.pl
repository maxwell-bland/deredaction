#!/usr/bin/perl

# find_bad_redactions.pl -- Given a PDF, returns a list of the pages containing bad redactions and the text
# under/near the redaction rectangle.

# Written in 2011 by Timothy B. Lee, tblee@princeton.edu

# To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
# rights to this software to the public domain worldwide. This software is distributed without any warranty. 

# You should have received a copy of the CC0 Public Domain Dedication along with this software. If not,
# see <http://creativecommons.org/publicdomain/zero/1.0/>. 
use File::Spec;
BEGIN {
  my ($volume, $directory, $file) = File::Spec->splitpath(__FILE__);
  push @INC, $directory;
  push @INC, $directory."lib/";
}
if(@ARGV != 1)
{
  print "Usage: $0 <filename>\n";
  exit;
}

$filename = $ARGV[0];

require $directory."lib/parse_pdf_for_redactions.pl";

open(F, $filename);

my $content;

while(<F>) { $content .= $_; }

my $redactions = get_bad_redactions($content);

foreach(sort {$a <=> $b} keys(%$redactions))
{
  print $redactions->{$_};
}

