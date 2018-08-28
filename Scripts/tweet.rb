#!/usr/bin/ruby


require 'rubygems'
require 'serialport'
require 'RMagick'
include Magick

sel_array = [@sp]

COLS = 512
ROWS  = 48
COL_SIZE = 100
COL_DELAY = 3000
PORT="/dev/tty.wchusbserialfa130"
BAUD_RATE = 115200


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
      # return
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

sp = SerialPort.new(PORT, BAUD_RATE, 8, 1, SerialPort::NONE)
sleep(2)

# images = ["mi3.jpg", "mi4.jpg", "mi5.jpg"]

while(true) do

lines = []
lines[0] = ARGV[0]
# ARGV.each do |file|
#   File.open(file, "r") do |infile|
#       while (line = infile.gets)
#        lines << line
#       end
#   end
#
# end



canvas  = ImageList.new
lines.each do |l|
  canvas.new_image(1200, 400){ self.background_color = "black" }


  text = Draw.new
  text.font_family = 'helvetica'
  text.pointsize = 150;
  text.gravity = Magick::NorthGravity
  text.annotate(canvas, 0,0,0,0, l) {
        self.fill = 'white'
  }
  text.annotate(canvas, 0,0,0,140, l) {
        self.fill = 'white'
  }
end

# canvas.display





  canvas.each do |i|
    #Reset carriage to home
    puts sp.write("F\n")
    waitForAck(sp)

    #Set column size
    puts sp.write("S#{COL_SIZE}\n")
    waitForAck(sp)

    #Set speed
    puts sp.write("D#{COL_DELAY}\n")
    waitForAck(sp)


    #Generate command line of greyscale values for each columm for each image

    img = i.resize(COLS,ROWS)

    img.colorspace = GRAYColorspace
    cnt = 0



    COLS.times do |c|
      cmd_line = ""
      ROWS.times do |r|
        src_pixel = img.get_pixels(c,r,1,1)[0].red/16
        # printf("PIX: %04d\n",src_pixel)
        # cmd_line << "%04d " % (r * 128)
        src_pixel /= 16
         cmd_line << "%02x " % src_pixel
      end
      #puts("CMD: #{cmd_line.gsub(/0000/, '    ')}")
      sp.write("G #{cmd_line}\n")
      waitForAck(sp)
    end

    #Move away from the image - this is for photographic purposes.
    # cmd_line = ""
    # 10.times do |m|
    #   ROWS.times do |r|
    #     cmd_line << "0000 "
    #   end
    #   sp.write("G #{cmd_line}\n")
    #   waitForAck(sp)
    # end


    #Scan forwards
    # COLS.times do |x|
    #     sp.write("C#{pix_array[x]}\n")
    #     printf("SENT:%d\n",pix_array[x])
    #     waitForAck(sp)
    #   end
    # sleep(1)
    # #Scan backwards
    # COLS.times do |x|
    #    sp.write("B#{pix_array[COLS-x]}\n")
    #    printf("SENT: %d\n",pix_array[COLS-x])
    #    waitForAck(sp)
    #  end
   end
end