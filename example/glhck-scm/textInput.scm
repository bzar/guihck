(define text-input-box
  (composite rectangle
    (prop 'width 256)
    (prop 'height 20)
    (prop 'color (bound '(input focus) 
      (lambda (focus)
        (if focus '(255 255 255) '(200 200 200)))))
    (alias 'text-color 'input 'color)
    (alias 'text 'input 'text)
    (alias 'font 'input 'font)
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
    (align 'center)
    (prop 'spacing 32)
    (text-input-box (id 'input-1))
    (text-input-box (id 'input-2)
      (prop 'text "Foo Bar Baz!"))
    (text-input-box (id 'input-3)
      (prop 'font "fonts/DejaVuSans.ttf"))
    (text-input-box (id 'input-4))))
  
(focus! (find-element 'input-1))