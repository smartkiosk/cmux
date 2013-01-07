module CMUX
  class MUX
    def initialize(device)
      @connection = CMUX::Connection.new
      @connection.open device

      io = CMUX::IO.open_tty "/dev/ttyUSB0"
      begin
        # Magic!

        # Modem will lose synchronization if previous session was interrupt
        # in middle of frame.
        2.times do
          # Send 'AT' to ensure proper autobaud after DTR rise.
          io.write "AT\r"

          # Terminate GSM 07.10 advanced multiplexer.
          # Will be ignored by modem in the AT mode.
          io.write "\x7e\x03\xef\xc3\x01\x70\x7e"
          io.flush

          # Read any crap from the buffer
          until ::IO.select([ io ], [], [], 0.2).nil?
            io.read_nonblock 2048
          end
        end

        chatter = CMUX::ModemChatter.new io

        all_done = false

        chatter.command("+CMUX=1,0,5,31") do |resp|
          if resp.failure?
            raise "CMUX failed: #{resp.error}"
          else
            all_done = true
          end
        end

        until all_done
          CMUX::ModemChatter.poll chatter
        end
      ensure
        io.close
      end

      @connection.activate
    end

    def close
      @connection.close
    end

    def allocate(channel)
      Channel.new @connection.open_port(channel), channel, self
    end

    def close_port(channel)
      @connection.close_port channel
    end
  end
end
