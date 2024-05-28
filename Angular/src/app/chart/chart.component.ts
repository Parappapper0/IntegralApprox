import { Component, OnInit, Input } from '@angular/core';
import { ChartModule } from 'primeng/chart';


@Component({
  selector: 'app-chart',
  standalone: true,
  imports: [ChartModule],
  templateUrl: './chart.component.html',
  styleUrl: './chart.component.scss'
})
export class ChartComponent implements OnInit {
  data: any;

  options: any;
  
  @Input() myFx!: string;

  ngOnInit() {

      const documentStyle = getComputedStyle(document.documentElement);
      const textColor = documentStyle.getPropertyValue('--text-color');
      const textColorSecondary = documentStyle.getPropertyValue('--text-color-secondary');
      const surfaceBorder = documentStyle.getPropertyValue('--surface-border');
      
      this.data = {
          labels: [],
          datasets: [{
              data: [],
              label: 'f(x)',
              tension: 0.4,
              pointStyle: false,
              borderColor: documentStyle.getPropertyValue('--blue-500'),
              
          }]
      };

      this.options = {
        maintainAspectRatio: false,
        aspectRatio: 1.6,
        spanGaps: true,
        plugins: {
            legend: {
                labels: {
                    color: textColor
                }
            }
        },
        elements:{
          point:{
              borderWidth: 0,
              radius: 10,
              backgroundColor: 'rgba(0,0,0,0)'
          }
      },
        scales: {
            x: {
                ticks: {
                    color: textColorSecondary,
                    autoskip:true,
                },
                grid: {
                    color: surfaceBorder
                }
            },
            y: {
                ticks: {
                    color: textColorSecondary,
                    autoskip:true,
                },
                grid: {
                    color: surfaceBorder
                }
            }
        }
      };

this.generateValues(this.functionTranslator(this.myFx))
  }

  functionTranslator(functionToTranslate : string){
    let f = functionToTranslate;
    f = f.replace("√", "Math.sqrt");
    f = f.replace("π", "Math.PI");
    f = f.replace("e", "Math.E");
    f = f.replace("ln", "Math.log");
    f = f.replace(/(\d+)\^(\d+)/g, 'pow($1, $2)').replace(/\b(sin|cos|tan)(([^)]+))/g, 'Math.$1($2)')
    return f;
  }

  
  generateValues(f : string){
   
    for (let index = 0; index < 40; index++) {
      this.data.labels[index]= index-15;
      this.data.datasets[0].data[index] = eval(f.replace("x",(index).toString()))
     
      
      
    }

    console.log(this.data.labels)
    console.log(this.data.datasets[0])
  }
}
