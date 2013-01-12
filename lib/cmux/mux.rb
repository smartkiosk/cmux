module CMUX
  class MUX
    def initialize(device)
      begin
        @connection = CMUX::Connection.new
        @connection.open device

        io = CMUX::IO.open_tty device
        begin
          # Magic!

          # Modem will lose synchronization if previous session was interrupted
          # in the middle of frame, and it will resync only at start of second
          # frame.
          2.times do
            # Send 'AT' to ensure proper autobaud after DTR rise in the AT mode.
            io.write "AT\r"

            # Terminate GSM 07.10 simple multiplexer.
            # Will be ignored by modem in the AT mode.
            io.write "\xf9\x03\xef\x05\xc3\x01\xf2\xf9"
            io.flush

            # Read any crap from the buffer
            until ::IO.select([ io ], [], [], 0.2).nil?
              io.read_nonblock 2048
            end
          end

          chatter = CMUX::ModemChatter.new io

          all_done = false

          chatter.command("&D2+IPR=115200;+CMUX=0", 5) do |resp|
            if resp.failure?
              raise "CMUX failed: #{resp.error}"
            else
              all_done = true
            end
          end

          until all_done
            CMUX::ModemChatter.poll [ chatter ]
          end
        ensure
          io.close
        end

        @connection.activate
      rescue Exception => e
        @connection.close

        raise e
      end
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
