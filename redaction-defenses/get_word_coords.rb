#!/usr/bin/env ruby
=begin
Uses PTSFormatter to retrieve an x_i, x_f and y for each word on each line.
=end

require_relative './PDF-Text-Analysis/parsing/pts_formatter'

pts_formatter = pts_parse_f(ARGV[0])

pts_formatter.words(true).each do |w|
  puts "#{w['word']}\t#{w['xi']}\t#{w['xf']}\t#{w['y']}"
end
