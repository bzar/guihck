(create-elements!
  (rectangle
    (align 'center)
    (prop 'width (bound '(parent width) (lambda (w) (/ w 2))))
    (prop 'height (bound '(parent height) (lambda (h) (/ h 2))))
    (prop 'color '(20 20 20))
    
    (text
      (align 'top-left 5)
      (prop 'text "top left"))
      
    (text
      (align 'bottom-left 5)
      (prop 'text "bottom left"))
      
    (text
      (align 'top-right 5)
      (prop 'text "top right"))
      
    (text
      (align 'bottom-right 5)
      (prop 'text "bottom right"))
      
    (text
      (align 'left 5)
      (align 'vertical-center)
      (prop 'text "center left"))
      
    (text
      (align 'right 5)
      (align 'vertical-center)
      (prop 'text "center right"))
      
    (text
      (align 'top 5)
      (align 'horizontal-center)
      (prop 'text "top center"))
      
    (text
      (align 'bottom 5)
      (align 'horizontal-center)
      (prop 'text "bottom center"))
      
    (text
      (align 'center)
      (prop 'text "center"))
      
    ))