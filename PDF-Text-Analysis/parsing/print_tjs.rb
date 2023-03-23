#!/usr/bin/env ruby
require_relative 'pts_formatter'
require_relative 'pdf_line'

# PARSE PTS
formatter = PTSFormatter.new
`#{File.dirname(__FILE__)}/../pts '#{ARGV[0]}' 1`.split("\n").each do |l|
  formatter.add_line l
end

formatter.get_lines.each do |line|
  puts line['raw_adjs']
end
