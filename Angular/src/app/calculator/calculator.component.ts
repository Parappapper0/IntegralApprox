import { Component, Output, EventEmitter, Input, OnInit } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import {MatGridListModule} from '@angular/material/grid-list';
import {MatButtonModule} from '@angular/material/button';

@Component({
  selector: 'app-calculator',
  standalone: true,
  imports: [RouterOutlet, MatGridListModule, MatButtonModule],
  templateUrl: './calculator.component.html',
  styleUrl: './calculator.component.scss'
})
export class CalculatorComponent implements OnInit{

  @Output() emitCodeExpression = new EventEmitter<string>();
  @Output() emitExpression = new EventEmitter<string>();
  
  @Input()
  inputExpression!: string; //this is what the user sees
  @Input()
  inputCodeExpression!: string; //this is what the code sees

  expression : string = "";
  codeExpression : string = "";
  lastOperation : string = "";
  amIdecimal : boolean = false;

  ngOnInit(){
    this.expression = this.inputExpression;
    this.codeExpression = this.inputCodeExpression;
    
  }

  editExpression = (operation : string) => {
    

    if(operation.match(/[ ) ]/)){
      let openBrackets = 0;
      let closedBrackets = 0;
      for(let i = 0; i<this.codeExpression.length; i++){
        if(this.codeExpression.charAt(i).match(/[ ( ]/))
          openBrackets++
          if(this.codeExpression.charAt(i).match(/[ ) ]/))
          closedBrackets++ 
      }
      if(openBrackets - closedBrackets-1 < 0 ){//check if the user try to put a closed bracket without having opened one
        alert("Syntax Error")
        
        return;
      } else if (this.codeExpression.length-1 - this.codeExpression.indexOf("(") == 0){ //check if in the brackets there is something
        alert("Syntax Error")
  
        return;
      }
    }              
      
    if((!this.lastOperation.match(/[0-9]/) && operation === ",") || 
      (this.lastOperation.match(/[ l ( ^ √ ] , /) && operation.match(/[ l ( ^ √ , ]/))){
      alert("Syntax Error");

      return;
    }

    if(this.lastOperation.match(/[0-9 e π ) x ]/) && operation.match(/[s c t S C T e π x l ( √ ]/) ) 
      this.codeExpression += "*";

    if (operation == "d")
      this.codeExpression = this.codeExpression.substring(0, this.codeExpression.length - 1);
    else if (operation.match(/[l √ s c t S C T]/)){
      this.codeExpression += operation+"(";
      this.lastOperation = "(";
    }
      
    else
      this.codeExpression += operation;
    
    if(operation == "d" && this.codeExpression.length > 1)

      this.lastOperation = this.codeExpression.charAt(this.codeExpression.length-1);
    else
      this.lastOperation = operation;
    this.updateExpression();
    
  }

  updateExpression = () => {
    this.expression = this.codeExpression;
    this.expression = this.expression.replaceAll("s", "sin");
    this.expression = this.expression.replaceAll("c", "cos");
    this.expression = this.expression.replaceAll("t", "tan");
    this.expression = this.expression.replaceAll("l", "ln");
    this.expression = this.expression.replaceAll("S", "asin");
    this.expression = this.expression.replaceAll("C", "acos");
    this.expression = this.expression.replaceAll("T", "atan");
  }


  syntaxCheck(){
    let openBrackets = 0;
    let closedBrackets = 0;
    for(let i = 0; i<this.codeExpression.length; i++){
      if(this.codeExpression.charAt(i).match(/[ ( ]/))
        openBrackets++
      if(this.codeExpression.charAt(i).match(/[ ) ]/))
        closedBrackets++ 
    }

    if(openBrackets - closedBrackets > 0 ){//check if the user try to put a closed bracket without having opened one
      alert("Syntax Error")
      
      return true;
    }
    return false;
  }

  done(){

    if(!this.syntaxCheck()){
      this.emitCodeExpression.emit(this.codeExpression);
      this.emitExpression.emit(this.expression);
    }
      console.log(this.codeExpression)  
  }
}

