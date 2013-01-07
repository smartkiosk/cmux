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
      @unsolicited = Hash.new do |hash, key|
        hash[key] = Set.new
      end

      command "E1V1+CMEE=2", 2
    end

    def command(command, timeout = nil, &block)
      submit_now = @command_queue.empty?
      cmd = ModemCommand.new(command, self, timeout, &block)
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

    def desired_timeout
      return nil if @command_queue.empty?

      command = @command_queue.first
      return nil if command.timeout.nil?

      now = DateTime.now.to_time

      remaining = command.issued_at + command.timeout - now
      if remaining < 0
        remaining = 0
      end

      remaining
    end

    def run_periodic
      remaining = desired_timeout

      if remaining == 0
        @have_running_command = false
        complete_first "timeout"
      end
    end

    def self.poll(devices, timeout_limit = nil)
      timeouts = devices.map(&:desired_timeout)
      timeouts << timeout_limit
      timeouts.reject! &:nil?
      read, write, except = ::IO.select devices.map(&:io), [], [], timeouts.min

      unless read.nil?
        read.each do |io|
          devices.find { |dev| dev.io == io }.poll
        end
      end

      devices.each(&:run_periodic)
    end

    def poll
      begin
        @buffer += @io.read_nonblock 4096
        on_read
      rescue Errno::EINTR, ::IO::WaitReadable
      end
    end

    def subscribe(type, receiver)
      @unsolicited[type].add receiver
    end

    def unsubscribe(type, receiver)
      @unsolicited[type].delete receiver
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
          if line == "OK" || line == "CONNECT"
            @have_running_command = false
            completed = true
            complete_first
          elsif line == "ERROR" || line == "NO CARRIER" || line == "NO DIALTONE" || line == "BUSY"
            @have_running_command = false
            completed = true
            complete_first line
          elsif line =~ /^\+CME ERROR: (.*)$/
            @have_running_command = false
            completed = true
            complete_first $1
          else
            @response << line
          end
        elsif @command_queue.any? && line == "AT#{@command_queue.first.command}"
          @have_running_command = true
        elsif !line.empty?
          on_unsolicited line
        end
      end

      submit if completed
    end

    def on_unsolicited(line)
      if line =~ /^\+([A-Z]+): (.*)$/
        type, body = $1, $2
      else
        return
      end

      list = []
      chunk = ""
      state = :void

      body.chars do |char|
        case state
        when :void
          case char
          when ','
            list << chunk
            chunk = ""

          when '"'
            state = :string

          else
            chunk << char
          end

        when :string
          case char
          when '"'
            state = :void

          when '\\'
            state = :escape

          else
            chunk << char
          end

        when :escape
          chunk << char
          state = :string
        end
      end

      list << chunk

      @unsolicited[type].each do |receiver|
        receiver.unsolicited type, list
      end
    end

    def complete_first(error = nil)
      response = ModemCommandResponse.new(error, @response)
      request = @command_queue.shift
      @response = []

      request.complete response
    end
  end
end
