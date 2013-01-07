module CMUX
  class ModemChatter
    attr_reader :io

    def initialize(io)
      @io = io
      @command_queue = []
      @buffer = ""
      @response = []
      @unprocessed_lines = []
      @have_running_command = true # because echo can be disabled before init string

      command "E1V1+CMEE=2"
    end

    def command(command, &block)
      submit_now = @command_queue.empty?
      cmd = ModemCommand.new(command, self, &block)
      @command_queue.push cmd

      submit if submit_now

      cmd
    end

    def submit
      return if @command_queue.empty?

      cmd = @command_queue.first
      cmd.start

      @io.write "AT#{cmd.command}\r\n"
    end

    def self.poll(*devices)
      read, write, except = ::IO.select devices.map(&:io)

      read.each do |io|
        devices.find { |dev| dev.io == io }.poll
      end
    end

    def poll
      begin
        @buffer += @io.read_nonblock 4096
        on_read
      rescue Errno::EINTR, ::IO::WaitReadable
      end
    end

    private

    def on_read
      chunk = @buffer.split "\r\n", -1
      @buffer = chunk.pop
      @unprocessed_lines += chunk

      completed = false

      while @unprocessed_lines.any?
        line = @unprocessed_lines.shift
        line.rstrip!

        if @have_running_command
          if line == "OK"
            @have_running_command = false
            completed = true
            complete_first
          elsif line == "ERROR"
            @have_running_command = false
            completed = true
            complete_first "error"
          elsif line =~ /^+CME ERROR: (.*)$/
            @have_running_command = false
            completed = true
            complete_first $1
          else
            @response << line
          end
        elsif @command_queue.any? && line = "AT#{@command_queue.first.command}"
          @have_running_command = true
        else
          on_unsolicited line
        end
      end

      submit if completed
    end

    def on_unsolicited(line)
      puts "unsolicited: #{line}"
    end

    def complete_first(error = nil)
      response = ModemCommandResponse.new(error, @response)
      request = @command_queue.shift
      @response = []

      request.complete response
    end
  end
end
