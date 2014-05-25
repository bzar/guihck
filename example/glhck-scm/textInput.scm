(define (text-input . args)
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
  (apply
    (composite text (list 'on-char append-char!
                          'on-key handle-key!)
      (mouse-area (list 'width '(bind (observe 'parent 'width) identity)
                        'height '(bind (observe 'parent 'height) identity)
                        'on-mouse-down (lambda (b x y) (focus! (parent))))))
    args))


(create-elements!
  (text-input '(id input)))
  
(focus! (find-element 'input))