module Types;

use v6;

enum Type is export < HashMap Integer Keyword List String Symbol Vector > ;

class Value is export {
    has Type $.type;
    has $.value;
}
