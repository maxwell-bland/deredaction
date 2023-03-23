use lib './lib';

# Written in 2011 by Timothy B. Lee, tblee@princeton.edu

# To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
# rights to this software to the public domain worldwide. This software is distributed without any warranty.

# You should have received a copy of the CC0 Public Domain Dedication along with this software. If not,
# see <http://creativecommons.org/publicdomain/zero/1.0/>.

use render_callback;

sub get_text_rects
{
  my $tree = shift;
  my $pagenum = shift;

  return () if(!defined($tree));

  render_callback->init($pagenum);

  $tree->render("render_callback");

  my @list = render_callback->getList;

  return render_callback->getList;
}

1;

