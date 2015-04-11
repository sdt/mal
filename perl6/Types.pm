module Types;

use v6;

enum Type is export <
    Constant HashMap Integer Keyword List String Symbol Vector
>;

class Value is export {
    has Type $.type;
    has $.value;
}

constant true  is export = 'true';
constant false is export = 'false';
constant nil   is export = 'nil';
