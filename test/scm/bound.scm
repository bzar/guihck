(import (rnrs (6)))

(create-elements!
  (item
    (id 'item-1)
    (prop 'a 0)
    (prop 'b 1)
    (prop 'value (bound '(this a this b) /))))
    
(define (display-all . things) (for-each display things))

(define (test id value)
  (begin
    (display-all id ": " (get-prop (find-element id) 'value) " = " value "\n")
    (assert (= (get-prop (find-element id) 'value) value))))

(test 'item-1 0)

(set-prop! (find-element 'item-1) 'a 5)

(test 'item-1 5)

(set-prop! (find-element 'item-1) 'b 5)

(test 'item-1 1)

(set-prop! (find-element 'item-1) 'a 10)

(test 'item-1 2)

