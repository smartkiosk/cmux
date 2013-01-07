#!/usr/bin/env ruby

require "erb"

def configure(src)
  @build   = Dir::pwd
  @src     = src
  @prefix  = "#{@build}/dst"


  File.write "#{@build}/Makefile", ERB.new(File.read("#{@src}/Makefile.in")).result(binding)
end