(import (rnrs (6)))

(create-elements!
  (item
    (id 'item-1)
    (prop 'value 1))
  (item
    (id 'item-2)
    (prop 'value (bound '(item-1 value)
                        (lambda (v) (+ v 1)))))
  (item
    (id 'item-3)
    (prop 'value (bound '(item-2 value)
                        (lambda (v) (+ v 1))))))

(define (display-all . things) (for-each display things))

(define (test id value)
  (begin
    (display-all id ": " (get-prop (find-element id) 'value) " = " value "\n")
    (assert (= (get-prop (find-element id) 'value) value))))
    
(test 'item-1 1)
(test 'item-2 2)
(test 'item-3 3)
 
(set-prop! (find-element 'item-1) 'value 2)
 
(test 'item-1 2)
(test 'item-2 3)
(test 'item-3 4)
 
(set-prop! (find-element 'item-2) 'value 4)
 
(test 'item-1 2)
(test 'item-2 4)
(test 'item-3 5)

(set-prop! (find-element 'item-3) 'value 6)
 
(test 'item-1 2)
(test 'item-2 4)
(test 'item-3 6)
