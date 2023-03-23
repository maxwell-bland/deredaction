#!/usr/bin/env ruby
=begin
Prints out sets of characters/spaces output from c-src/pts that are on
the same line and have a large shift between them (>500 quanta) as
two arrays, representing the x_i,y_i,x_f,y_f device space coordinates.
=end

# Defines the starting and ending coordinates of a letter/space
class Coord
  attr_accessor :x_i, :x_f, :y_i, :y_f,
  :x_i, :x_f, :y_i, :y_f, :quanta

  def initialize(x_i, y_i, x_f, y_f, quanta)
    @x_i = x_i.to_f
    @y_i = y_i.to_f
    @x_f = x_f.to_f
    @y_f = y_f.to_f
    @quanta = quanta.to_f
  end

  def <=>(o)
    if @y_i == o.y_i
      @x_i <=> o.x_i
    else
      @y_i <=> o.y_i
    end
  end

  def to_s
    "[#{@x_i},#{@y_i},#{@x_f},#{y_f}]"
  end

  def on_same_line?(o)
    @y_i == o.y_i && @quanta == o.quanta
  end
end

# seen_ascii helps us avoid bullet-points when counting shifts
seen_ascii = lc = quanta = nil
ch_cnt = 0
ARGF.each { |line|
  case line
  when /^QUANTA\s+-?(.*?)\n/
    quanta = $1.to_f
  # Note here that including spaces in this computation is critical, because
  # it avoids false positives that emerge during large inter-word shifts
  when /^(CH|SP)\[(.*?)\].*?\s+(?:(\S+)\s+)(?:(\S+)\s+)(?:(\S+)\s+)(?:(\S+)\s+)/
    cc = Coord.new($3, $4, $5, $6, quanta)
    if (lc && cc.on_same_line?(lc))
      if ch_cnt > 1 && seen_ascii && (cc.x_i - lc.x_f) / quanta > 500
        puts "#{lc.to_s}\t#{cc.to_s}"
      end
    else
      seen_ascii = false
      ch_cnt = 0
    end

    lc = cc

    if !seen_ascii && $1 == "CH" && $2.ascii_only?
      seen_ascii = true
    end

    ch_cnt += 1
  end
}
