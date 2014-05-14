guihck
======

Overview
--------

Guihck is a user interface framework based on GNU Guile scheme interpreter. 
It can be used to create user interfaces for several graphical backends and 
does not depend on any graphics library.

Interfaces are created by composing elements similarly to QtQuick. 
Elements may depend on the used graphics library, and as such any
graphical elements need to be reimplemented for each graphical backend.


Element
-------

Elements are the basic building blocks of guihck. Element types are registered through
the C interface and can be combined in script to create new elements. For example,
a simple button element can be constructed using rectangle, text and mouse area elements.
Once defined, the elements can be freely used in user interfaces.

Elements interface with each other using properties. Each element can have any number of 
properties. Properties may contain data (like element coordinates or color), procedures,
or special values like property bindings or property aliases.

Some element properties may be special for some element types. For instance properties
"x", "y", "width" and "height" are by convention used to communicate an element's location
and size on screen. Each element type may choose to communicate with the user interface
using any number of properties.

Some element types may also depend on other elements having some properties defined. 
The provided row element, for example, depends on other elements having the previously
mentioned "x", "width" and "height" properties with number values.

Elements may have any number of child elements. Child elements typically refer to their
parent element for layout origin and may be otherwise managed by it. The row element
changes the "x" property of its children so that they are positioned next to each other.

Property
--------

Properties can have any values GNU Guile defines. The element type may impose restrictions
upon which types of values it responds to, but generally any property can be of any type.

Properties may define procedures to be executed at certain times. The procedures are
expressed as quoted lists. For example, the mouse area element has a special property
"on-mouse-down", which determines the action taken when a mouse button is pressed on top
of the mouse area element.

Every property has a special change handler procedure property. Their names have the 
form of "on-<property name>". For example, the change handler procedure property for
"x" is "on-x".

Elements may have a value for the special "id" property that can be used in locating them.
Whenever the an element is searched, the search is started from the current element.
If the current element does not match the searched id, the element's direct ancestors
are searched. If the id is not found there either, the element's descendants are searched
breadth-first. Finally, the element's siblings are searched. The behaviour following from
not finding an element with the specified id is currently unspecified.

Properties may also have special values, such as property aliases or property bindings.

Property aliases make properties work as if their value was actually stored in another 
property, usually in another element. Assigning a new value to a property alias actually
assigns a new value to the aliased property. Similarly getting the value of a property alias
actually returns the value of the aliased property.

Property bindings define the value of the property as as the value of a procedure.
Whenever the value of a property binding is requested, the provided binding function is
executed and its return value is returned as result. Unlike property aliases, assigning
to a property binding just changes the value of the property.

Context
-------

Context holds all the information about a guihck user interface. It provides 
a C interface for interacting with the interface from the host application's side.
Through the interface the application may, for instance, register element types 
and functions, create and remove elements, execute scripts and provide input and 
timing events.

The context also has an element stack based interface used by the script interface.
This is generally not the most convenient way of interacting with the context from
the host application, but may be of use.

In summary: all communication with a guihck interface from the host application 
and element types goes through a context.


Simple example
--------------

The below example shows many of guihck's features. The used elements are provided
in either the generic guihckElements or the glhck-specific guihckGlhck module.


    (define button
      (composite rectangle '(width 64
                            height 32
                            color (240 240 240)
                            on-clicked (alias (find-element 'mouse-area) on-mouse-down))
        (text '(text "Button!" 
                x (bind (/ (- (get-prop (parent) 'width) (get-prop 'width)) 2))
                y (bind (/ (- (get-prop (parent) 'height) (get-prop 'height)) 2)) 
                color (24 24 24)))
        (mouse-area '(id mouse-area
                      width (bind (get-prop (parent) 'width))
                      height (bind (get-prop (parent) 'height))
                      on-mouse-down (set-prop! (parent) 'color '(255 255 255))
                      on-mouse-up (set-prop! (parent) 'color '(240 240 240))))))

    (create-elements!
      (row '(x 100 y 100 spacing 16)
        (column '(spacing 8)
          (button '())
          (button '(width 128)))
        (column '(spacing 8)
          (button '())
          (button '(height 64))
          (button '(on-clicked (set-prop! (parent) 'color '(255 0 0)))))))

The code first defines button as a composite element that has a rectangle element as its root.
The rectangle has some default properties, including one property alias.

The rectangle has two children: a text element with the content "Button!" and a mouse-area. 
The text element's x and y coordinates are bound to the parent rectangle so that the text 
stays in the center. The mouse-area gets its width and height from the parent rectangle in
the same way.

The mouse-area has two handler procedures: on-mouse-down and on-mouse-up. They set the parent 
rectangle's color to reflect the state of the mouse. Notice, that the rectangle's on-clicked
property aliases the mouse-area's on-mouse-down property. This means that if a button
defines an on-clicked property, it is actually assigned to the mouse-area's on-mouse-down.

Next comes the actual element instantiation. The code defines an element row at coordinates
(100, 100) with element spacing of 16. The row contains two columns, which in turn contain
buttons. Some of the buttons are content with using jsut hte default values, some set different 
widths and heights. The last button redefine's what the button does, changing the color
it gets when clicked.