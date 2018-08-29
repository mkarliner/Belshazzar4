#!/usr/bin/ruby


require 'rubygems'
require 'serialport'
require 'rmagick'
include Magick

sel_array = [@sp]

COLS = 512
ROWS  = 48
COL_SIZE = 100
COL_DELAY = 1500
PORT="/dev/ttyUSB0"
BAUD_RATE = 115200
FONT_SIZE = 90


cnt = 0

def waitForAck(sp, cmd, silent)
  msg = ""
  
  while(true)
    msg = sp.gets
    if(msg[0] == "O" && msg[1] == "K") 
	if(!silent)
          puts("#{cmd}: ACKED #{msg}")
	end
      return
    else
      puts("Unknown response #{msg.inspect}")
      exit(1)
      # return
    end
  end
end






def set_pix_array(img, pix_array)

  img = img.resize(COLS,ROWS)
  #img = img.change_geometry(cols) {|cols, rows, img| img.resize!(cols, rows)}
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
sleep(1)

# images = ["mi3.jpg", "mi4.jpg", "mi5.jpg"]

while(true) do

lines = []
lines[0] = ARGV[0] ? ARGV[0] : " "
lines[1] = ARGV[1] ? ARGV[1] : " "
# ARGV.each do |file|
#   File.open(file, "r") do |infile|
#       while (line = infile.gets)
#        lines << line
#       end
#   end
#
# end
puts("Line 1: #{lines[0]} #{lines[0].length}")
puts("Line 2: #{lines[1]} #{lines[1].length}")


canvas  = ImageList.new
lines.each do |l|
	yoffset = 0
  canvas.new_image(1200, 300){ self.background_color = "black" }


  text = Draw.new
  text.font_family = 'helvetica'
  text.pointsize = FONT_SIZE;
  text.gravity = Magick::NorthWestGravity
  text.annotate(canvas, 0,0,0,40, lines[0].upcase) {
        self.fill = 'white'
  }
  text.annotate(canvas, 0,0,0,170, lines[1].upcase){
        self.fill = 'white'
  }
end

# canvas.display





  canvas.each do |i|
    #Reset carriage to home
    sp.write("F\n")
    waitForAck(sp, "F", false)

    #Set column size
    sp.write("S#{COL_SIZE}\n")
    waitForAck(sp,"S#{COL_SIZE}", false)

    #Set speed
    sp.write("D#{COL_DELAY}\n")
    waitForAck(sp, "D#{COL_DELAY}", false)


    #Generate command line of greyscale values for each columm for each image

    img = i.resize(COLS,ROWS)
#	img=i

    img.colorspace = GRAYColorspace
    cnt = 0



    (COLS-100).times do |c|
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
      waitForAck(sp, "G", true)
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
