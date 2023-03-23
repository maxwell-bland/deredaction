#!/usr/bin/env ruby
=begin
Script that takes in a PDF as the first command line argument and then a set of
left bottom right top coordinates at 300 DPI. We then generate a png of the input PDF
page with a 50% opacity red box indicating coordinates land.
=end
rects = [];
STDIN.each { |l|
  next if l.strip == '-'
  coords = l.split(/\s+/).map { |c| c.to_f / 72 * 300 }
  left = coords[0]
  bottom = coords[1]
  right = coords[2]
  top = coords[3]
  rects.append " -draw 'rectangle " +
    "#{left - 6},#{top - 6},#{right + 6},#{bottom + 6}' ";
  }

if rects != []
  IO.popen(
      "convert -gravity southwest -density 300 " +
      " -stroke '#0000ffff' -strokewidth 5 -fill '#0000ff40' " +
      rects.join('') +
      "\"#{ARGV[0]}\" \"#{ARGV[1]}\"" , "r"
    ) { |c| c.read }
else
  $stderr.write "NO RECTS #{ARGV[0]}\n"
end
