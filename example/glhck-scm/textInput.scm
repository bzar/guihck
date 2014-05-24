(define text-input
  (composite text (list 'on-char (lambda (c) 
                          (set-prop! 'text 
                            (list->string 
                              (append 
                                (string->list 
                                  (get-prop 'text)) (list c))))))
    (mouse-area (list 'width '(bind (observe 'parent 'width) identity)
                      'height '(bind (observe 'parent 'height) identity)
                      'on-mouse-down (lambda (b x y) (focus! (parent)))))))
                    
                    
(create-elements!
  (text-input '(id input)))
  
(focus! (find-element 'input))