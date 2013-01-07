module CMUX
  class Connection

    class Pointer < FFI::AutoPointer
      def self.release(pointer)
        Library.cmux_destroy pointer
      end
    end

    def initialize
      buffer = FFI::Buffer.new :pointer, 1
      context = Library.cmux_create buffer

      if context.null?
        ptr = buffer.read_pointer
        begin
          error = ptr.read_string
          raise "MUX creation failed: #{error}"
        ensure
          Library.cmux_free ptr
        end
      end

      @handle = Pointer.new context
    end

    def open(device)
      status = Library.cmux_open @handle, device
      if status == -1
        raise Library.cmux_error(@handle)
      end

      nil
    end

    def activate
      status = Library.cmux_activate @handle
      if status == -1
        raise Library.cmux_error(@handle)
      end

      nil
    end

    def close
      @handle.free
      @handle = nil
    end

    def open_port
      buffer = FFI::Buffer.new :pointer, 1
      status = Library.cmux_open_port @handle, buffer
      if status == -1
        raise Library.cmux_error(@handle)
      end

      pointer = buffer.read_pointer
      begin
        pointer.read_string
      ensure
        Library.cmux_free pointer
      end
    end
  end
end
