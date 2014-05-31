(define text-input-box
  (composite rectangle
    (prop 'width 256)
    (prop 'height 20)
    (prop 'color (bound '(input focus) 
      (lambda (focus)
        (if focus '(255 255 255) '(200 200 200)))))
    (prop 'text-color '(alias (find-element 'input) 'color))
    (prop 'text '(alias (find-element 'input) 'text))
    (prop 'init 
      (lambda () 
        (bind 'focus 
          (lambda (focus) 
            (if focus (focus! (find-element 'input)))))))
    (text-input
      (id 'input)
      (prop 'color '(0 0 0))
      (prop 'size (bound '(parent height)))
      (prop 'width (bound '(parent width)))
      (prop 'height (bound '(parent height))))))


(create-elements!
  (column
    (prop 'x 100)
    (prop 'y 30)
    (prop 'spacing 32)
    (text-input-box (id 'input-1))
    (text-input-box (id 'input-2)
      (prop 'text "Foo Bar Baz!"))
    (text-input-box (id 'input-3))
    (text-input-box (id 'input-4))))
  
(focus! (find-element 'input-1))