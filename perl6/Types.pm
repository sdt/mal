module Types;

use v6;

enum Type is export <
    Constant HashMap Integer Keyword List String Symbol Vector
>;

class Value is export {
    has Type $.type;
    has $.value;

    method Str { return $.type ~ ':' ~ $.value }
}

class TypeError is Exception { }
class ArityError is Exception { }

constant true  is export = 'true';
constant false is export = 'false';
constant nil   is export = 'nil';

sub make-hash(@items) is export {
    if @items.elems % 2 != 0 {
        die ArityError.new('hashmap requires an even number of args');
    }
    my %hash = @items.map(sub ($k, $v) {
        given $k.type {
            when Keyword {
                $k.value => $v;
            }
            when String {
                $k.value.perl => $v;
            }
            default {
                die TypeError.new('hash keys must be keywords or strings');
            }
        }
    });
    return Value.new(:type(HashMap), :value(%hash));
}
