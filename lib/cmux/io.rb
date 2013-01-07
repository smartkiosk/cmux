module CMUX
  module IO
    def self.open_tty(filename)
      buffer = FFI::Buffer.new :pointer, 1

      fd = Library.cmux_open_device filename, buffer
      if fd == -1
        ptr = buffer.read_pointer
        begin
          error = ptr.read_string
        ensure
          Library.cmux_free ptr
        end

        raise error
      end

      FFI::IO.for_fd fd, "r+"
    end
  end
end
