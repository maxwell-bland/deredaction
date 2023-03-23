#!/usr/bin/env ruby

# Represents a single line in a pdf
class PDFLine
  attr_reader :cur_c

  def initialize(g_state)
    @chars = []
    @uchars = []
    @adjs = []
    @raw_adjs = []
    @x_is = []
    @cur_c = 0
    @g_state = g_state
  end

  def x_i
    @x_is[0]
  end

  def x_f
    @x_is[-1]
  end

  def get_x_before(x)
    (0..(@x_is.length-1)).each do |i|
      if @x_is[i] == x and i != 0
        return @x_is[i - 1]
      elsif @x_is[i] == x 
        return x
      end
    end
  end

  def words(use_space_heuristic = false)
    # Returns an array of words in the line.
    # If use_space_heuristic is true, then words are split also 
    # by considering whether the corresponding raw adjustment is
    # greater than 100 quanta
    space_char_width = 0
    word_boundaries = []
    first_char = false
    word_start = -1
    word = ''
    (0..@chars.length - 1).each do |i|
      if @chars[i] == ' ' and i != @chars.length - 1 and space_char_width == 0  and use_space_heuristic
        space_char_width = @x_is[i + 1].to_f - @x_is[i].to_f
      end

      if @chars[i] != ' ' and not first_char
        first_char = true
        word_start = @x_is[i]
      end

      # get absolute value of raw adjustment at this point
      if first_char && (@chars[i] =~ /[\s[:punct:]]/)
        word_boundaries.append([word, word_start, @x_is[i]]) if word.length >= 2
        first_char = false
        word = ''
        # handle space heuristic
      elsif first_char && @raw_adjs[i].to_f.abs > space_char_width and use_space_heuristic
        word += @chars[i]
        word_boundaries.append([word, word_start, @x_is[i]]) if word.length >= 2
        first_char = false
        word = ''
      elsif first_char
        word += @chars[i]
      end
    end

    if word_start != -1
      i = @chars.length - 1
      word_boundaries.append([word, word_start, @x_is[i]]) if word.length >= 2
    end

    word_boundaries
  end

  def new_char(type, meta)
    if type == 'SP'
      # NOTE: DOES NOT HANDLE TABS!
      [' ', meta, 0 - @g_state.space_space]
    else
      [meta, format('%04X', meta.ord), 0 - @g_state.letter_space]
    end
  end

  def add_char(meta, unicode, radj)
    @chars.append meta
    @uchars.append unicode
    @raw_adjs.append radj
    @adjs.append 0
    @cur_c += 1
  end

  def add_meta(type, meta, x)
    if meta =~ /-?\d+\.?\d*/ && type == 'SH' && @cur_c - 1 >= 0
      @adjs[@cur_c - 1] = meta.to_f
      @raw_adjs[@cur_c - 1] += @adjs[@cur_c - 1] * @g_state.hscale
    else
      add_char(*new_char(type, meta))
      @x_is.append x
    end
  end

  def add_break(pts_l)
    if @g_state.quant != 0
      x0, _, x1 = pts_l.split(/\s+/)[1, 3]
      sh = (x1.to_f - x0.to_f) / @g_state.quant
      if @cur_c - 1 >= 0
        # Breaks not counted in meta character adjustments
        # @adjs[@cur_c - 1] -= sh
        @raw_adjs[@cur_c - 1] -= sh
      end
    end
  end

  def reprs
    strs = []
    [[@chars, @adjs], [@chars, @raw_adjs], [@uchars, @raw_adjs], [@chars, Array.new(@chars.size, 0)]].map do |ps|
      str = '[('
      ps.transpose.each { |p| str += p[1] != 0 ? "#{p[0]})#{p[1].round 6}(" : p[0] }
      str += ')]'
      strs.append str
    end
    strs
  end

  def get_prefix_inner(x, field)
    str = '[('
    [@chars, field, @x_is].transpose.each do |_t|
      if _t[2].to_f.round(3) == x.to_f.round(3)
        str += _t[0]
        break
      end

      str += _t[1] != 0 ? "#{_t[0]})#{_t[1].round 6}(" : _t[0]
    end
    str += ')]'
    str
  end

  def get_prefix_meta(x)
    get_prefix_inner(x, @adjs)
  end

  def get_prefix(x)
    get_prefix_inner(x, @raw_adjs)
  end

  def contains_x(x_o)
    @x_is.each do |x|
      return true if x.to_f.round(3) == x_o.to_f.round(3)
    end
    false
  end

  def contains_x_exact(x)
    @x_is.each do |xo|
      return true if xo == x
    end
    false
  end

  def get_suffix_inner(x, field)
    str = '[('
    x_hit = false
    [@chars, field, @x_is].transpose.each do |_t|
      x_hit = true if _t[2].to_f.round(3) == x.to_f.round(3)

      if x_hit
        str += _t[1] != 0 ? "#{_t[0]})#{_t[1].round 6}(" : _t[0]
      end
    end
    str += ')]'
    str
  end

  def get_suffix_meta(x)
    get_suffix_inner(x, @adjs)
  end

  def get_suffix(x)
    get_suffix_inner(x, @raw_adjs)
  end
end
