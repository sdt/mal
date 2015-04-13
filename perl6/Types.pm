module Types;

use v6;

enum Type is export <
    Constant HashMap Integer Keyword List String Symbol Vector
>;

constant true  is export = 'true';
constant false is export = 'false';
constant nil   is export = 'nil';

class Value is export {
    has Type $.type;
    has $.value;

    method Str { return $.type ~ ':' ~ $.value }

    my %constants = true  => Value.new(:type(Constant), :value(true)),
                    false => Value.new(:type(Constant), :value(false)),
                    nil   => Value.new(:type(Constant), :value(nil)),
                    ;

    method true     { %constants<true>  }
    method false    { %constants<false> }
    method nil      { %constants<nil>   }

    method Constant(Str $name) { %constants{$name} }
}

class MAL-Exception is Exception is export {
    has $.reason;
    method message { return $.reason }
    method new($reason) { self.bless(:$reason) }
}

class TypeError  is MAL-Exception { }
class ArityError is MAL-Exception { }

sub make-hash(@items) is export {
    if @items.elems % 2 != 0 {
        die ArityError.new('Hashmap requires an even number of args');
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
                die TypeError.new('Hash keys must be keywords or strings');
            }
        }
    });
    return Value.new(:type(HashMap), :value(%hash));
}

sub make-form(Str $symbol, *@list) is export {
    Value.new(:type(List), :value(
        Value.new(:type(Symbol), :value($symbol)),
        @list));
}