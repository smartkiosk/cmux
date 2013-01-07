module CMUX
  class Channel
    attr_reader :device
    attr_reader :port

    def initialize(device, port, mux)
      @device = device
      @port = port
      @mux = mux
    end

    def open(&block)
      io = IO.open_tty @device
      if block_given?
        begin
          yield io
        ensure
          io.close
        end
      else
        io
      end
    end

    def close
      @mux.close_port @port
    end
  end
end
