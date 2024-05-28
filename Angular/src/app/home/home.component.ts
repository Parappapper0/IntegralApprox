import { Component } from '@angular/core';
import {  FormsModule, ReactiveFormsModule,  } from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatInputModule} from '@angular/material/input';
import {MatIconModule} from '@angular/material/icon';
import {MatButtonToggleModule} from '@angular/material/button-toggle';
import { CalculatorComponent } from '../calculator/calculator.component';
import { CommonModule } from '@angular/common';
import { requestService } from '../services/request.service';
import { GraphService } from '../services/graph.service';
import { RouterOutlet, RouterLink } from '@angular/router';
import { ChartComponent } from '../chart/chart.component';

@Component({
  selector: 'app-home',
  standalone: true,
  imports: [RouterLink, ChartComponent, RouterOutlet,MatButtonModule, MatIconModule, MatButtonToggleModule, ReactiveFormsModule, MatInputModule, FormsModule, CalculatorComponent, CommonModule],
  templateUrl: './home.component.html',
  styleUrl: './home.component.scss'
})
export class HomeComponent {

  constructor(private requestService : requestService, public graphService : GraphService) { }
  
  show : boolean = false;
  showGraph : boolean = false;

  resultIsReady : boolean = false;

  codeExpression : string = "";

  time : number = 10;
  value : number = 200;

  errors : boolean = false;
  boundAError : boolean = false;
  boundBError : boolean = false;
  nError : boolean = false;
  functionError : boolean = false;
  methodError : boolean = false;


  methods = ["Rettangoli", "Triangoli", "Parabole"];
  aria_labels = ["right", "center", "left"];

  form = {
    boundA : 0,
    boundB : 1,
    n : "",
    function : "",
    method : ""
  }

  updateExpression(chosenFunction: string){
    this.form.function = chosenFunction;
    this.show=false;

    console.log(this.cFunctionTranslator(this.form.function))
  }

  updateCodeExpression(codeExpression : string){
    this.codeExpression = codeExpression;
  }
 
  formControl(){
      this.boundAError = this.form.boundA < 0;
      this.boundBError = this.form.boundB < 0;
      this.nError = this.form.n == "".trim() || parseInt(this.form.n) < 0;
      this.functionError = this.form.function == "".trim();
      this.methodError = this.form.method == "".trim();
      this.errors = this.form.boundA < 0 || this.form.boundB < 0 || this.form.n == "".trim() ||  parseInt(this.form.n) < 0 || this.form.function == "".trim() || this.form.method == "".trim();
  }


  cFunctionTranslator(functionToTranslate : string){
    let f = functionToTranslate;

    f = f.replace("√", "sqrt");
    f = f.replace("π", "M_PI");
    f = f.replace("e", "M_E");
    f = f.replace("ln", "log");
    f = f.replace(/(\d+)\^(\d+)/g, 'pow($1, $2)')
    console.log(f)
    return f;
  }

  onSubmit(){
    this.formControl()

    if(this.boundAError || this.boundBError || this.nError || this.functionError || this.methodError)
      return;


    
    this.requestService.send(this.form.boundA.toString(), this.form.boundB.toString(), this.form.n, this.cFunctionTranslator(this.form.function), this.form.method).subscribe(res => {

      console.log(res)
      let prova : payload = res as any;
      
      this.time = prova.time;
      this.value = prova.value;
      
      
    });
    this.resultIsReady=!this.resultIsReady;
  }
}

interface payload{
  value : number;
  time : number;
}