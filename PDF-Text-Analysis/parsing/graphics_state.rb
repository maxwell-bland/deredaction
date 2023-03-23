#!/usr/bin/env ruby

# contains the current line spacing information
class GraphicsState
  def initialize
    @hscale = 1
    @cspace = @wspace = @quant = 0
  end

  attr_reader :quant, :hscale

  def set_lambs
    { 'QUANTA' => ->(val) { @quant = val },
      'HSCALE' => ->(val) { @hscale = val },
      'CSPACE' => ->(val) { @cspace = val },
      'WSPACE' => ->(val) { @wspace = val } }
  end

  def space_space
    (@cspace + @wspace) * 1000 * @quant
  end

  def letter_space
    @cspace * 1000 * @quant
  end
end
