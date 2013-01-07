module CMUX

  module Library
    extend FFI::Library

    ffi_lib File.expand_path(File.dirname(__FILE__) + "/../libcmux.so")

    attach_function :cmux_create,      [ :buffer_out ], :pointer
    attach_function :cmux_destroy,     [ :pointer ], :void
    attach_function :cmux_error,       [ :pointer ], :string
    attach_function :cmux_open,        [ :pointer, :string ], :int
    attach_function :cmux_activate,    [ :pointer ], :int
    attach_function :cmux_open_port,   [ :pointer, :int, :buffer_out ], :int
    attach_function :cmux_close_port,  [ :pointer, :int ], :int
    attach_function :cmux_free,        [ :pointer ], :void
    attach_function :cmux_open_device, [ :string, :buffer_out ], :int
  end

end
