module Types;

use v6;

class malEnv { ... }

class malException is Exception is export {
    has $.reason;
    method message { return $.reason }
    method new($reason) { self.bless(:$reason) }
}

class ArityError    is malException is export { }
class InternalError is malException is export { }
class NotFound      is malException is export { }
class RuntimeError  is malException is export { }
class TypeError     is malException is export { }

class malValue is export {
    has $.value;
}

class malBoolean is malValue is export {
    method new(Bool $value) { self.bless(:$value) }
}
constant malTrue  is export = malBoolean.new(True);
constant malFalse is export = malBoolean.new(False);

class malBuiltIn is malValue is export {
    method new(Code $value) { self.bless(:$value) }
}

class malNilValue is malValue is export {
    method new() { self.bless(:value('nil')) }
}
constant malNil is export = malNilValue.new;

class malHash is malValue is export {
    method new(Hash $value) { self.bless(:$value) }
}

class malInteger is malValue is export {
    method new(Int $value) { self.bless(:$value) }
}

class malKeyword is malValue is export {
    method new(Str $value) { self.bless(:$value) }
}

class malSequence is malValue is export { }

class malList is malSequence is export {
    method new(*@value) { self.bless(:@value) }
}

class malString is malValue is export {
    method new(Str $value) { self.bless(:$value) }
}

class malSymbol is malValue is export {
    method new(Str $value) { self.bless(:$value) }
}

class malVector is malSequence is export {
    method new(Array $value) { self.bless(:$value) }
}

class malLambda is malValue is export {
    # $.value is the $body
    has Str     @.args;
    has malEnv  $.env;

    method new(malSequence $args, malValue $body, malEnv $env) {
        # Check all args are symbols, and extract the values;
        my @args = $args.value.map: -> malSymbol $arg { $arg.value };

        # Check for duplicates (including multiple ampersands)
        die RuntimeError.new('Args must be unique')
            unless @args.elems == @args.unique.elems;

        # There's only zero or one ampersand now. Check it's in the right place.
        my $ampIndex = @args.first-index('&');
        if $ampIndex.defined && $ampIndex != @args.elems - 2 {
            die RuntimeError.new('Slurpy & must be second-last argument');
        }
        self.bless(:value($body), :@args, :$env);
    }
}

class malMacro is malValue is export {
    method new(malLambda $value) { self.bless(:$value) }
}

class malEnv is export {
    has %.data;
    has malEnv $.outer;

    method set(Str $key, malValue $value) {
        %.data{$key} = $value;
        return $value;
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
            die NotFound.new("'$key' not found");
        }
        return $env.data{$key};
    }

    method bind(@bindings, @values) {
        # $bindings has already been checked to be valid
        my $n = @bindings.elems;
        if $n >= 2 && @bindings[$n - 2] eq '&' {
            my $key = @bindings[$n - 1];
            my $val = malList.new(@values[$n - 2 .. * - 1]);
            self.set($key, $val);

            $n -= 2;
        }
        for ^$n -> $i {
            self.set(@bindings[$i], @values[$i]);
        }
    }
}

sub make-hash(@items) is export {
    if @items.elems % 2 != 0 {
        die ArityError.new('Hashmap requires an even number of args');
    }
    my %hash = @items.map(sub ($k, $v) {
        given $k {
            when malKeyword {
                $k.value => $v;
            }
            when malString {
                $k.value.perl => $v;
            }
            default {
                die TypeError.new('Hash keys must be keywords or strings');
            }
        }
    });
    return malHash.new(%hash);
}

sub make-form(Str $symbol, *@list) is export {
    return malList.new(malSymbol.new($symbol), @list);
}

sub make-bool(Bool $value) is export {
    return $value ?? malTrue !! malFalse;
}

sub is-true(malValue $v) is export {
    return !($v ~~ malNil) && !($v ~~ malFalse);
}
