module Printer;

use v6;

use Types;

sub pr-str(Value $ast) is export {
    given $ast.type {
        when HashMap {
            return '{' ~ print-items($ast.value) ~ '}';
        }
        when List {
            return '(' ~ print-items($ast.value) ~ ')';
        }
        when Vector {
            return '[' ~ print-items($ast.value) ~ ']';
        }
        when String {
            return $ast.value.perl;
        }
        default {
            return $ast.value;
        }
    }
}

sub print-items(@items) {
    return @items.map({ pr-str($_) }).join(' ');
}
