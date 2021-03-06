#!/usr/bin/ruby


require 'rubygems'
require 'serialport'
require 'RMagick'
include Magick

sel_array = [@sp]

COLS = 128
ROWS  = 16
COL_SIZE = 50


cnt = 0

def waitForAck(sp)
  msg = ""

  while(true)
    msg = sp.gets
    if(msg == "OK\r\n")
      # puts("ACKED")
      return
    else
      puts("Unknown response #{msg.inspect}")
      return
    end
  end
end






def set_pix_array(img, pix_array)

  img = img.resize(COLS,ROWS)
  img = img.flip
  COLS.times do |c|
    dest_pixel = 0
    ROWS.times do |r|
      src_pixel = img.get_pixels(c,r,1,1)[0]
      puts(src_pixel.intensity)
      if(src_pixel.intensity > 10000)
        puts("Set pixel #{c} \t#{r} to \t#{src_pixel.intensity}")
        dest_pixel = dest_pixel | 1
      else
        dest_pixel = dest_pixel & 0xfffe
      end
      dest_pixel = dest_pixel << 1  
      # puts pix_array.inspect
    end 
    puts("DST_PIX: #{dest_pixel}")
    pix_array << dest_pixel
  end
end

#Start of main program

puts("START:")

sp = SerialPort.new("/dev/tty.usbserial-A600aulh", 9600, 8, 1, SerialPort::NONE)
sleep(5)

images = ["mi3.jpg", "mi4.jpg", "mi5.jpg"]



while(true) do

  
  
  images.size.times do |i|
    #Reset carriage to home
    puts sp.write("F\n")
    waitForAck(sp)
    
    #Set column size 
    puts sp.write("S#{COL_SIZE}\n")
    waitForAck(sp)
    
    #Set pixel arrary for each image
    pix_array = []
    puts("FILE: #{images[i]}")
    il = ImageList.new(images[i])
    set_pix_array(il[0], pix_array)
    cnt = 0
  
    #Scan forwards
    COLS.times do |x|
       sp.write("C#{pix_array[x]}\n")
       printf("SENT:%d\n",pix_array[x])
       waitForAck(sp)
     end
    # sleep(1)
    # #Scan backwards
    # COLS.times do |x|
    #    sp.write("B#{pix_array[COLS-x]}\n")
    #    printf("SENT: %d\n",pix_array[COLS-x])
    #    waitForAck(sp)
    #  end
   end
end


