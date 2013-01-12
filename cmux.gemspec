# -*- encoding: utf-8 -*-
require File.expand_path('../lib/cmux/version', __FILE__)

Gem::Specification.new do |gem|
  gem.authors       = ["Sergey Gridasov"]
  gem.email         = ["grindars@gmail.com"]
  gem.description   = %q{Ruby bindings for CMUX, GSM 07.10 multiplexer}
  gem.summary       = %q{Ruby bindings for CMUX, GSM 07.10 multiplexer}
  gem.homepage      = ""

  gem.files         = `git ls-files`.split($\)
  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.name          = "cmux"
  gem.require_paths = ["lib"]
  gem.version       = CMUX::VERSION

  gem.add_dependency 'ffi'
end
