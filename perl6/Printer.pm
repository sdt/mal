module Printer;

use v6;

use Types;

sub pr-str(malValue $ast, Bool $readably) is export {
    given $ast {
        when malHash {
            return '{' ~ print-hash($ast.value, $readably) ~ '}';
        }
        when malList {
            return '(' ~ print-items($ast.value, $readably) ~ ')';
        }
        when malVector {
            return '[' ~ print-items($ast.value, $readably) ~ ']';
        }
        when malString {
            return $readably ?? $ast.value.perl !! $ast.value;
        }
        when malBoolean {
            return $ast.value ?? 'true' !! 'false';
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
