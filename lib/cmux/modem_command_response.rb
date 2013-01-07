module CMUX
  class ModemCommandResponse
    attr_reader :error, :response

    def initialize(error, body)
      @error = error
      @response = body
    end

    def success?
      @error.nil?
    end

    def failure?
      !success?
    end
  end
end
