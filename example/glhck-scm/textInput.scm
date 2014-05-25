(define (text-input-gen)
  (define (append-char! c)
    (set-prop! 'text
      (list->string
        (append
          (string->list
            (get-prop 'text)) (list c)))))
  (define (pop-char!)
    (let ((current-text (get-prop 'text)))
      (if (> (string-length current-text) 0)
        (set-prop! 'text (string-drop-right current-text 1)))))

  (define (handle-key! key scancode action mods)
    (cond ((and (= key (keyboard 'backspace))
                (not (eq? action 'release)))
          (pop-char!))))

  (composite text 
    (prop 'on-char append-char!)
    (prop 'on-key handle-key!)
    (mouse-area 
      (prop 'width (bound '(parent width) identity))
      (prop 'height (bound '(parent height) identity))
      (prop 'on-mouse-down (lambda (b x y) (focus! (parent)))))))
      
(define text-input (text-input-gen))



(create-elements!
  (text-input (id 'input)))
  
(focus! (find-element 'input))