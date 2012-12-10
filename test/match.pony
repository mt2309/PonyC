/*

* Object constants

An immutable constructor with no side effects (no actors created or messages sent) creates a constant.
Constants are ambient. They can be created at compile time.

* Patterns

Don't have to do decomposition, because the result of a match is a typed object that we can examine.
Do need type matching.
Do need value matching.
For value matching, it can be combined with type matching by matching a type, a type with a value, or a type with
a subset of values - with the others being ignored when doing structural equality.

partial T
{
  // magic: each field's type is based on T's fields' types
  _fields:Field|\Field|Undefined

  // magic: all field names are optional parameters with default value Undefined
  new( a = Undefined, b = Undefined, ... )

  // magic: all field names are optional parameters with default value \Field
  new all( a = \A, b = \B, ... )

  // magic: string representation of the name of T
  function typename()->( "T" )

  // magic: returns Undefined if the field doesn't exist
  function field( name:String )->( _fields( name ) )

  // magic: throws if the field doesn't exist or value isn't an acceptable type
  function setfield( name:String, value:Any ) throws { _fields( name ) = value }

  // magic: iterates over fields
  function fields()->( MapIter[T]( _fields ) )
}

trait Pattern
{
  function typename()->( String )
  function field( name:String )->( Any )
  function setfield( name:String, value:Any ) throws
  function fields()->( MapIter[T] )
}

object Conjunction[A, B]
{
  var _a:A
  var _b:B

  new( _a, _b )
  function equals( to:Any )->( (to ?= _a) | (to ?= _b) )
}

object Disjunction[A, B]
{
  var _a:A
  var _b:B

  new( _a, _b )
  function equals( to:Any )->( (to ?= _a) & (to ?= _b) )
}

* Structural equality

This becomes a "general structural equality" problem, with the additional twist of "partial equality", where only
some fields are examined for equality.

Structural comparison between:
  two objects
  an object and a pattern
  two patterns
  two object and a pattern for partial equality

true if:
  type is T or \T
  for all fields in T
    (pattern.field == Undefined) | (this.field == Undefined) | (this.field ?= that.field)

* What about traits as opposed to objects?

It's easy to do type matching on a trait, but value matching is weird: traits have no values. Would we ever want
to be able to match on some "common fields" in two different types?

* It's All The Same

Pattern matching, structural equality, cloning, pretty printing, serialisation, factories. All the same.

Structural equality: compare only defined fields
Pattern matching: structural equality on partial object
Cloning: clone defined fields, reference undefined fields
Pretty printing: print only defined fields
Serialisation: write only defined fields
Deserialisation: read defined fields, assign from undefined fields from companion object
Factories: call constructors on partial objects (field definition doesn't matter)

* Reflection

Reflection is only available for 'this', so it doesn't give access to the fields
of an object unless an object hands out a mirror. There are no default methods
that hand out a mirror.

mirror:\T = reflect( pattern = \T.all() )

The mirror only includes the fields in the pattern. The values of the fields are
captured when the mirror is created. It is not recursive: it contains fields,
not mirrors of fields. This protects fields: have to call a method on them that
hands back a mirror.

* Pretty printing

  Use a pattern to indicate what should be printed:
  foo.print( \Foo( field1 = \Type1.all() ) )
    only prints field1
  foo.print( \Foo( field1 = \Type1( field2 = \Type2.all() ) ) )
    only prints field1, and only prints field2 from field1
  foo.print()
    defaults to foo.print_pattern()

*/

// since matching is structural equality, can use literals
// including disjunction?
function fib( n:U64 )->( r:U64 )
{
  match n
  {
    case 0|1 { r = 0 }
    case 2 { r = 1 }
    case { r = fib( n - 1 ) + fib( n - 2 ) }
  }
}

/* Red, Blue and Green are not defined elsewhere, so they are singleton types
use a notation for 'this really is just a singleton type'?
*/
type Color:Red|Blue|Green

/* Or we can define them
any object with no members is a singleton type
  singleton types must not have explicit constructors
  they get an implicit constructor: new(){}
*/
trait Named { function name()->( r:String ) }

object Red is Named { function name()->( "red" ) }
object Blue is Named { function name()->( "blue" ) }
object Green is Named { function name()->( "green" ) }

// every type in our ADT must implement Named
// we can now call Named methods on Color
type Color:Red|Blue|Green is Named

var a = Label
var b = Label.blue()
var c = Label( color = Blue )

object Label
{
  var _text:String
  var _color:Color

  new( _text = "", _color = Red )
  new blue( _text = "", _color = Blue )

  function is_blue()->( _color == Blue )
  function color_name()->( _color.name() )
}

function blue_labels( ws:Set[Widget] )->( r = Set[Label] )
{
  for w in ws
  {
    match w
    {
      case \Label( _color = Blue ) as n { r.add( n ) }
      case {}
    }
  }
}

object Math
{
  function pi()->( F64( 3.141592 ) )
}
