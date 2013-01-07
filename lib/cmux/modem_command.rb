module CMUX
  class ModemCommand
    attr_reader :state, :command, :timeout, :issued_at

    def initialize(command, chatter, timeout, &block)
      @command = command
      @chatter = chatter
      @timeout = timeout
      @issued_at = DateTime.now.to_time
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
