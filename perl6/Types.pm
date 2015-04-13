module Types;

use v6;

class malValue is export {
    has $.value;

    method Str { return $.type ~ ':' ~ $.value }
}

class malBoolean is malValue is export {
    method new(Bool $value) { self.bless(:$value) }
}
constant malTrue  is export = malBoolean.new(True);
constant malFalse is export = malBoolean.new(False);

class malBuiltIn is malValue is export {
    method new(Sub $value) { self.bless(:$value) }
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

class malException is Exception is export {
    has $.reason;
    method message { return $.reason }
    method new($reason) { self.bless(:$reason) }
}

class TypeError  is malException { }
class ArityError is malException { }

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
