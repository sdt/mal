module Environment;

use v6;

use Types;

class NotFound is MAL-Exception { }

class Env is export {
    has %.data;
    has Env $.outer;

    method set(Str $key, Value $value) {
        %.data{$key} = $value;
    }

    method find(Str $key) {
        if %.data{$key}:exists {
            return self;
        }
        if $.outer.defined {
            return $.outer.find($key);
        }
        return Value.nil;
    }

    method get(Str $key) {
        my $env = self.find($key);
        if $env ~~ Value.nil {
            die NotFound.new("\"$key\" not found");
        }
        return $env.data{$key};
    }
}
