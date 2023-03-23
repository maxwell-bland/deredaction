#!/usr/bin/env ruby
require 'tempfile'
DIR=File.expand_path(File.dirname(__FILE__))

io = IO.popen("python3.8 -m xray '#{ARGV[0]}'")
xray_res = io.readlines
io.close
if !xray_res.empty?
  xray_res.each do |i|
    f = Tempfile.create
    fpath = File.basename(f.path + ".pdf")
    io = IO.popen("pdftk '#{ARGV[0]}' cat #{i.strip} output '#{fpath}'" + 
                  " && #{DIR}/../trivial_redaction_loc.sh #{fpath}")
    locate_out = io.read
    io.close
    if locate_out != ""
      puts xray_res
      puts "---"
      puts "#{i.strip}"
      puts locate_out
      puts ""
    end

    `rm #{fpath}`
  end
end
