module CMUX
  class ModemCommand
    attr_reader :state
    attr_reader :command

    def initialize(command, chatter, &block)
      @command = command
      @chatter = chatter
      @block = block
      @state = :queued
    end

    # modemchatter api

    def start
      @state = :executing
    end

    def complete(response)
      @state = :complete

      @block.call response unless @block.nil?
    end
  end
end
