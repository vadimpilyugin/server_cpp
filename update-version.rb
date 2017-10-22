class String
  # @private
  def colorize(i)
    return "\x1b[1;#{i}m#{self}" # \x1b[0m"
  end
  # When printed, sets the output stream color to green
  # @return [String] output same string with control characters
  def green
    return colorize(32)
  end
  def white
    return colorize(37)
  end
  # Escape all % signs
  def perc_esc
    self.index('%') ? self.gsub('%','%%') : self
  end
end

# Class used for printing debug messages. Also used for checking assertions
#
class Printer

  LOG_EVERY_N = 1

  class Chars
    DELIM = ": "
    CR = "\r"
    LF = "\n"
    TAB = "\t"
  end

  class Messages
    DEBUG = 'Debug'
    EMPTY = ''
  end

  class Colors
    GREEN = 'green'
    WHITE = 'white'
  end

  def self.generic_print(
    who:, msg:, in_place:false, params:{}, \
    who_color:Colors::WHITE, msg_color:Colors::WHITE, delim:Chars::DELIM,
    log_every_n: false, line_no: 0)

    if !log_every_n || log_every_n && line_no % LOG_EVERY_N == 0
      printf(who.to_s.public_send(who_color)+\
        Chars::DELIM+msg.to_s.public_send(msg_color)
      )
      if in_place
        printf Chars::CR
      else
        printf Chars::LF
        params.each_pair do |s1,s2|
          printf Chars::TAB+s1.to_s.perc_esc.white+\
            Chars::DELIM+s2.to_s.perc_esc.white+Chars::LF
        end
      end
    end
  end

  def self.debug(who: Messages::DEBUG, msg: Messages::EMPTY, \
    params: {}, in_place:false, log_every_n: false, line_no: 0)

    generic_print(
      msg:msg,
      who:who,
      in_place:in_place,
      log_every_n: log_every_n,
      line_no: line_no,
      params:params,
      who_color: Colors::GREEN,
      msg_color: Colors::WHITE
    )
  end
end


def dot_ver(ver)
  ((ver+0.0)/10).round(1)
end

# get version
File.read('Makefile') =~ /Server_v(?<ver>\d+)/
version = $~['ver'].to_i
new_version = version+1

# files to update
file_names = ['Makefile', 'config.cfg']

# modification data
regexes = [/Server_v\d+/, /HTTP Model Server v\d+\.\d+/]
strings = ["Server_v#{new_version}", "HTTP Model Server v#{dot_ver(new_version)}"]

# 
Printer::debug(who:"Update version", msg:"#{dot_ver(version)} ~> #{dot_ver(new_version)}")
file_names.each_with_index do |file_name,i|
  printf "Apply changes to <#{file_name}> Y/n? "
  s = gets.chomp
  next if s. != 'Y'.downcase
  text = File.read(file_name)
  new_contents = text.gsub(regexes[i], strings[i])
  # To merely print the contents of the file, use:
  puts new_contents

  # To write changes to the file, use:
  File.open(file_name, "w") {|file| file.puts new_contents }
end
