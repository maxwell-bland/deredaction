#!/usr/bin/perl -w

# remove_rectangles.pl -- remove rectangles from a PDF, producing a new PDF (named *.norects.pdf) in which all
# redaction-like rectangles are replaced by hollow red rectangles. Useful for seeing what is "under"
# the redaction rectangles in a file.

# Written in 2011 by Timothy B. Lee, tblee@princeton.edu

# To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
# rights to this software to the public domain worldwide. This software is distributed without any warranty.

# You should have received a copy of the CC0 Public Domain Dedication along with this software. If not,
# see <http://creativecommons.org/publicdomain/zero/1.0/>.

use CAM::PDF;
use CAM::PDF::Content;

if(@ARGV != 1)
{
  print "Usage: $0 <filename>\n";
  exit;
}

my $filename = $ARGV[0];

my $output_filename;

if($filename =~ /^(.*)\.pdf$/)
{
  $output_filename = $1.".norects.pdf";
}
else
{
  print "Error: filename must end in .pdf\n";
  exit;
}

require "lib/strip_polygons.pl";

open(F, $filename);

my $content;

while(<F>) { $content .= $_; }

my $pdf = CAM::PDF->new($content) || return ();

my $pages = $pdf->numPages();

my $error;

for(my $pagenum = 1; $pagenum <=$pages; $pagenum++)
{
  my $pagetree;
  eval
  {
    $pagetree = $pdf->getPageContentTree($pagenum)
  };

  if($error = $@)
  {
    warn "Caught error: $error";
  }
  else
  {
    my $new_pagetree = strip_polygons($pagetree);
    $pdf->setPageContent($pagenum, $new_pagetree->toString());
  }
}

$pdf->cleanoutput($output_filename);

