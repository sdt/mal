module Environment;

use v6;

use Types;

class NotFound is malException { }

class malEnv is export {
    has %.data;
    has malEnv $.outer;

    method set(Str $key, malValue $value) {
        %.data{$key} = $value;
    }

    method find(Str $key) {
        if %.data{$key}:exists {
            return self;
        }
        if $.outer.defined {
            return $.outer.find($key);
        }
        return malNil;
    }

    method get(Str $key) {
        my $env = self.find($key);
        if $env ~~ malNil {
            die NotFound.new("\"$key\" not found");
        }
        return $env.data{$key};
    }
}
