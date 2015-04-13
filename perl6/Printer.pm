module Printer;

use v6;

use Types;

sub pr-str(Value $ast, Bool $readably) is export {
    given $ast.type {
        when HashMap {
            return '{' ~ print-hash($ast.value, $readably) ~ '}';
        }
        when List {
            return '(' ~ print-items($ast.value, $readably) ~ ')';
        }
        when Vector {
            return '[' ~ print-items($ast.value, $readably) ~ ']';
        }
        when String {
            return $readably ?? $ast.value.perl !! $ast.value;
        }
        default {
            return $ast.value;
        }
    }
}

sub print-items(@items, $readably) {
    return @items.map({ pr-str($_, $readably) }).join(' ');
}

sub print-hash(%hash, $readably) {
    return %hash.pairs.map({
        $_.key ~ ' ' ~ pr-str($_.value, $readably)
    }).join(' ');
}
