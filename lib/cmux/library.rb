module CMUX

  module Library
    extend FFI::Library

    ffi_lib "cmux"

    attach_function :cmux_create,      [ :buffer_out ], :pointer
    attach_function :cmux_destroy,     [ :pointer ], :void
    attach_function :cmux_error,       [ :pointer ], :string
    attach_function :cmux_open,        [ :pointer, :string ], :int, :blocking => true
    attach_function :cmux_activate,    [ :pointer ], :int, :blocking => true
    attach_function :cmux_open_port,   [ :pointer, :int, :buffer_out ], :int, :blocking => true
    attach_function :cmux_close_port,  [ :pointer, :int ], :int, :blocking => true
    attach_function :cmux_free,        [ :pointer ], :void
    attach_function :cmux_open_device, [ :string, :buffer_out ], :int, :blocking => true
  end

end
