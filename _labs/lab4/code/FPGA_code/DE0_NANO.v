`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	LED,
	KEY
);


//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;
localparam BLACK = 8'b000_000_00;
//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;


////////////// LED ///////////////////////////////////////////
output 		    [7:0]		LED;


//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [40:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [40:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'd0;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign GPIO_0_D[0] = CLOCK_24_PLL;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
reg W_EN;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
wire 			CLOCK_24_PLL;
wire 			CLOCK_25_PLL;
wire  		CLOCK_50_PLL;

///////* INSTANTIATE YOUR PLL HERE *///////
team11PLL	team11PLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( CLOCK_24_PLL ),
	.c1 ( CLOCK_25_PLL),
	.c2 ( CLOCK_50_PLL )
);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(CLOCK_50),
	.clk_R(CLOCK_25_PLL), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(CLOCK_25_PLL),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(CLOCK_25_PLL),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.VSYNC(VSYNC),
	.HREF(HREF),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end


///////* Buffer Writer *///////
/*
integer i = 0;
integer j = 0;

always @(posedge CLOCK_24_PLL) begin
		W_EN = 1;
		
		X_ADDR = i;
		Y_ADDR = j;
		
		if (i < `SCREEN_WIDTH && (j % 10 == 0))
			pixel_data_RGB332 = RED;
		else
			pixel_data_RGB332 = BLACK;
			
		i = i+1;
			
		if (i == `SCREEN_WIDTH) begin
			i = 0;
			j = j+1;
			
			if (j == `SCREEN_HEIGHT) begin
				j = 0;
			end
		end
end
*/

///////* Downsampler *///////
reg[7:0] i = 0;
reg[7:0] j = 0;
reg      k = 0;

reg      h_flag = 0;
reg      v_flag = 0;

reg  [7:0]	input_1 = 8'd0;
reg  [7:0]	input_2 = 8'd0;
wire [7:0]	CAMERA_INPUT;
assign 		CAMERA_INPUT = {GPIO_1_D[23],GPIO_1_D[21],GPIO_1_D[19],GPIO_1_D[17],GPIO_1_D[15],GPIO_1_D[13],GPIO_1_D[11],GPIO_1_D[9]};
wire			P_CLOCK;
assign		P_CLOCK = GPIO_1_D[7];
wire			HREF;
assign		HREF = GPIO_1_D[5];
wire			VSYNC;
assign		VSYNC = GPIO_1_D[3];

always @(posedge P_CLOCK) begin

	if (VSYNC == 1'b1 && v_flag == 0) begin
		i = 0;
		j = 0;
		v_flag = 1;
	end
	
	else if (VSYNC == 1'b0) begin
	
		v_flag = 0;
		
		if (HREF == 1'b0 && h_flag == 0) begin
			k = 0;
			i = 0;
			j = j+1;
			h_flag = 1;
		end
		
		else if (HREF == 1'b1) begin
			h_flag = 0;
			
			X_ADDR = i;
			Y_ADDR = j;
			
			if (k == 0) begin
				input_1 = CAMERA_INPUT;
				k = 1;
				W_EN = 0;
			end
			
			else begin
				input_2 = CAMERA_INPUT;
				pixel_data_RGB332 = {input_2[7:5], input_2[2:0], input_1[4:3]};
				i = i+1;
				k = 0;
				W_EN = 1;
			end
		end
	end		
end

///////* Color Detection *///////
reg     LED_7;
reg     LED_6;
reg     LED_0;

assign   LED[7] = LED_7;
assign   LED[6] = LED_6;
assign   LED[0] = LED_0;

always @(*) begin
		if (RESULT == 3'b111) begin //turn on LED 7
			LED_7 = 1; 
			LED_6 = 1;
			LED_0 = 1;
		end
		else if (RESULT == 3'b111) begin //turn on LED 6
			LED_7 = 0;
			LED_6 = 1;
			LED_0 = 0;
		end
		else begin
			LED_7 = 0;
			LED_6 = 0;
			LED_0 = 1;
		end
end

	
endmodule 