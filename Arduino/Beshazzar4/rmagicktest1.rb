#!/usr/bin/ruby


require 'rubygems'
require 'serialport'
require 'RMagick'
include Magick


il = ImageList.new("MI.jpg")
img = il[0].resize(130,16)


130.times do |c|
  16.times do |r|
    pixel = img.get_pixels(c,r,1,1)[0]
    puts(pixel.intensity)
  end 
end
img.display



exit