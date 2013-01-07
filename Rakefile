#!/usr/bin/env rake

require "bundler/gem_tasks"
require "rake/extensiontask"

spec = Gem::Specification.load('cmux.gemspec')
Rake::ExtensionTask.new('libcmux', spec)
Rake::ExtensionTask.new('cmuxcontrold', spec)
