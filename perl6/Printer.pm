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
        when malLambda {
            return 'user-function: ' ~ print-lambda($ast, $readably);
        }
        when malAtom {
            return '(atom ' ~ pr-str($_.value, $readably) ~ ')';
        }
        when malMacro {
            return 'user-macro: ' ~ print-lambda($ast.value, $readably);
        }
        default {
            return $ast.value;
        }
    }
}

sub print-lambda(malLambda $ast, Bool $readably) {
    return '(' ~ $ast.args ~ ') => ' ~ pr-str($ast.value, $readably);
}

sub print-items(@items, Bool $readably) {
    return @items.map({ pr-str($_, $readably) }).join(' ');
}

sub print-hash(%hash, Bool $readably) {
    return %hash.pairs.map({
        $_.key ~ ' ' ~ pr-str($_.value, $readably)
    }).join(' ');
}
