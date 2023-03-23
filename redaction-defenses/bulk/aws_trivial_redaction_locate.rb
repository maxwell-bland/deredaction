#!/usr/bin/env ruby
# Reads in aws pdf paths from stdin and runs the locate method on
# each to find trivially broken redactions.
DIR=File.expand_path(File.dirname(__FILE__))

puts ARGV[0]
fname = ARGV[0].gsub("/", ".")
fname = fname[5,fname.length]

if not File.file?(fname)
  puts "FETCHING #{ARGV[0]}"
  `aws s3 cp '#{ARGV[0]}' '#{fname}' --no-sign-request`
end

File.open("#{fname}.tags", "w+") { |f|
  io = IO.popen("#{DIR}/xray_pass_trivial_redaction_locate.rb '#{fname}'");
  res = io.read
  if res != ""
    f.write(res);
    f.write("\n");
    io.close
    io = IO.popen("ls -alh #{fname}")
    f.write(io.read);
    f.write("\n");
  end
  io.close
}
`rm -f page.pdf`
