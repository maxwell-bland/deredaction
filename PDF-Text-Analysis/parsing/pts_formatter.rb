#!/usr/bin/env ruby
require 'set'
require_relative 'pdf_line'
require_relative 'graphics_state'

CHAR_DATA_REGEX = /^(CH|SH|SP)\[(.*?)\]\s/
# Formats pts input into various string types
class PTSFormatter
  def initialize(key_fonts = false)
    # Key fonts attaches a unique key based upon the object id to each font object.
    # used for properly resolving the correct width dictionary during width guessing
    @line_hash = {}
    @g_state = GraphicsState.new
    @key_fonts = key_fonts
  end

  def add_scale(pts_l)
    split_l = pts_l.split
    @g_state.set_lambs[split_l[0]].call(split_l[1].to_f) if @g_state.set_lambs.include? split_l[0]
  end

  def add_line(pts_l)
    text_meta = if pts_l =~ CHAR_DATA_REGEX
                  [Regexp.last_match(1), Regexp.last_match(2)]
                else
                  []
                end

    if text_meta.length != 0
      split_l = pts_l.split("\t").map {|v| v.strip}
      key = {"y" => split_l[2], "font" => split_l[-1]}
      @line_hash[key] = PDFLine.new(@g_state) unless @line_hash.include? key
      @line_hash[key].add_meta(*text_meta, split_l[1])
    elsif pts_l =~ /^TJ/
      split_l = pts_l.split("\t").map {|v| v.strip}
      key = {"y" => split_l[2], "font" => split_l[-1]}
      @line_hash[key] = PDFLine.new(@g_state) unless @line_hash.include? key
      @line_hash[key].add_break(pts_l)
    else
      add_scale(pts_l)
    end
  end

  def get_fonts
    fonts = Set.new
    @line_hash.each do |key, line|
      fonts.add key['font'].split('[')[0]
    end
    fonts
  end

  def get_font_line_hash(font)
    @line_hash.select do |k, _v|
      font_key = k['font'].split('[')[0]
      font == font_key
    end
  end

  def get_lines_font(font)
    get_lines_internal(get_font_line_hash(font))
  end

  def get_lines
    get_lines_internal @line_hash
  end

  def get_lines_internal(hash)
    lines = []
    hash.each do |key, line|
      next if line.cur_c.zero?

      begin
        line_dict = {}
        line_dict['text'] = line.reprs[3]
        line_dict['font_name'] = key['font'].split('[')[0]
        line_dict['meta_adjs'] = line.reprs[0]
        line_dict['raw_adjs'] = line.reprs[1]
        line_dict['y'] = key['y']
        line_dict['x_i'] = line.x_i
        line_dict['x_f'] = line.x_f
        lines.append line_dict
      rescue NoMethodError
      end
    end
    lines
  end

  def get_x_before(x, y)
    # Gets the xi value before a given character on
    # a line if there is one, else returns the given x
    @line_hash.each do |key, line|
      next if line.cur_c.zero?
      if key['y'] == y
        return line.get_x_before(x)
      end
    end
    return x
  end

  def line(x, y)
    @line_hash.each do |key, line|
      next if line.cur_c.zero?
      if key['y'] == y
        return line if line.contains_x_exact x
      end
    end
    return nil
  end

  def words(use_space_heuristic = false)
    words = []

    @line_hash.each do |key, line|
      next if line.cur_c.zero?

      line.words(use_space_heuristic).each do |w|
        words.append({ 'word' => w[0], 'xi' => w[1], 'xf' => w[2], 'y' => key['y'] })
      end
    end

    words
  end

  def get_prefix_meta(x, y)
    @line_hash.each do |key, line|
      next if line.cur_c.zero?

      return line.get_prefix_meta(x) if y == key['y']
    end
  end

  def get_suffix_meta(x, y)
    @line_hash.each do |key, line|
      next if line.cur_c.zero?

      return line.get_suffix_meta(x) if y == key['y']
    end
  end

  def get_prefix(x, y)
    @line_hash.each do |key, line|
      next if line.cur_c.zero?

      return line.get_prefix(x) if y == key['y']
    end
  end

  def get_font(x, y)
    @line_hash.each do |key, line|
      next if line.cur_c.zero?

      next unless y.to_f.round(3) == key['y'].to_f.round(3) && line.contains_x(x)

      font_key = key['font'].split('[')[0]
      font_key = font_key.split(',')[1] unless @key_fonts
      return font_key
    end
  end

  def get_suffix(x, y)
    @line_hash.each do |key, line|
      next if line.cur_c.zero?

      return line.get_suffix(x) if y == key['y']
    end
  end
end

def pts_parse_f(pdf_path)
  formatter = PTSFormatter.new(true)
  `#{File.dirname(__FILE__)}/../pts '#{pdf_path}' 1`.split("\n").each do |l|
    formatter.add_line l
  end
  formatter
end
