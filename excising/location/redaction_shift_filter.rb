#!/usr/bin/env ruby
=begin
This file takes in lines of pairs of tab-seperated [x_i,y_i,x_f,y_f] (72 DPI)
and a PDF as a command line argument and crops out 20 pixel tall 300 DPI pngs
using y_i as a midline, then prints left,bot,right,top if the PNG has <10% mean
pixel brightness. bottom and top are a half inch below and above the midline
(southwest gravity) and the metadata left and right are the starts of the characters
=end
require 'tempfile'

Tempfile.create { |tmpf|
  IO.popen("qpdf \"#{ARGV[0]}\" --pages \"#{ARGV[0]}\" #{ARGV[1]} -- #{tmpf.path}") { |c| c.read
    IO.popen("identify -format \"%H\" \"#{tmpf.path}\"", 'r') { |ident|
      y_dim = ident.read.to_f
      IO.popen(
        "convert -density 300 -quality 100 \"#{tmpf.path}\" \"#{tmpf.path}.jpg\""
      ) { |c0| c0.read
        STDIN.each { |line|
          boxes = line.split(/\s+/).map {|box| eval(box) }
          dpi_boxes = boxes.map { |l| l.map { |x| (x / 72) * 300 } }
          IO.popen(
            "convert -gravity northwest -crop " +
            "#{dpi_boxes[1][0] - dpi_boxes[0][2]}x20+" +
            "#{dpi_boxes[0][2]}+#{dpi_boxes[0][1] - 20}" +
            " \"#{tmpf.path}.jpg\" \"#{tmpf.path}\"" , "r"
          ) { |c1| c1.read
            IO.popen(
              "convert \"#{tmpf.path}\" -colorspace gray " +
              "-format \"%[fx:(mean)]\n\" info:", 'r'
            ) { |black_pixels|
              puts "#{boxes[0][0]} #{y_dim - boxes[1][1] - 32}" +
                   "#{boxes[1][0]} #{y_dim - boxes[1][1] + 32}" if
                black_pixels.read.to_f < 0.1;
            }
          }
        }
      }
      IO.popen("rm \"#{tmpf.path}.jpg\"")
    }
  }
}
