// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

`timescale 1ns/100ps
`default_nettype none
//
// This is the top module
// It interfaces to the LCD display and touch panel
module exercise1 (
	/////// board clocks                      ////////////
	input logic CLOCK_50_I,                   // 50 MHz clock
	
	/////// pushbuttons/switches              ////////////
	input logic[3:0] PUSH_BUTTON_I,           // pushbuttons
	input logic[17:0] SWITCH_I,               // toggle switches
	
	/////// 7 segment displays/LEDs           ////////////
	output logic[6:0] SEVEN_SEGMENT_N_O[7:0], // 8 seven segment displays
	output logic[8:0] LED_GREEN_O,            // 9 green LEDs
	output logic[17:0] LED_RED_O,             // 18 red LEDs
	
	/////// GPIO connections                  ////////////
	inout wire[35:0] GPIO_0                   // GPIO Connection 0 (LTM)
);

// Signals for LCD Touch Module (LTM)
// LCD display interface
logic 	[7:0]	LTM_R, LTM_G, LTM_B;
logic 			LTM_HD, LTM_VD;
logic 			LTM_NCLK, LTM_DEN, LTM_GRST;

// LCD configuration interface
wire 			LTM_SDA;
logic 			LTM_SCLK, LTM_SCEN;

// LCD touch panel interface
logic 			TP_DCLK, TP_CS, TP_DIN, TP_DOUT;
logic 			TP_PENIRQ_N, TP_BUSY;

// Internal signals
logic 			Clock, Resetn;
logic 	[2:0] 	Top_state;

// For LCD display / touch screen
logic 			LCD_TPn_sel, LCD_TPn_sclk;
logic 			LCD_config_start, LCD_config_done;
logic 			LCD_enable, TP_enable;
logic 			TP_touch_en, TP_coord_en;  //touch_en is when touch is sensed, coord_en is when coord change is detected
logic 	[11:0]	TP_X_coord, TP_Y_coord;

logic TP_touch_en_buf, TP_coord_en_buf;

logic 	[9:0] 	Colourbar_X, Colourbar_Y;
logic 	[7:0]	Colourbar_Red, Colourbar_Green, Colourbar_Blue;

logic 	[4:0] 	TP_position[7:0];

logic [2:0] block_colour_0, block_colour_0_buf;
logic [2:0] block_colour_1, block_colour_1_buf;
logic [2:0] block_colour_2, block_colour_2_buf;
logic [2:0] block_colour_3, block_colour_3_buf;
logic [2:0] block_colour_4, block_colour_4_buf;
logic [2:0] block_colour_5, block_colour_5_buf;
logic [2:0] block_colour_6, block_colour_6_buf;
logic [2:0] block_colour_7, block_colour_7_buf;

logic [15:0] clock_1_ms_div_count;
logic clock_1_ms, clock_1_ms_buf;

//recall logic 	[4:0] 	TP_position[7:0];
logic [3:0] BCD_count[3:0];
logic [4:0] touchDuration[7:0];

logic [3:0] block_index;
logic [3:0] block_index_buf;// track current and previous coordinates
logic inSameBlock; //check to see if in same block


logic initialTap;
logic isHeld;
logic sampleFlag;
logic sampleFlag_buf;
logic updateColour;
logic heldThreeSecs;

logic [1:0] VSYNC_UPDATE;

logic [27:0] three_sec_region_counter;
logic clear_3_sec_region_counter;


 

assign Clock = CLOCK_50_I;
assign Resetn = SWITCH_I[17];

assign LCD_TPn_sclk = (LCD_TPn_sel) ? LTM_SCLK : TP_DCLK;
assign LTM_SCEN = (LCD_TPn_sel) ? 1'b0 : ~TP_CS;
assign LTM_GRST = Resetn;

// Connections to GPIO for LTM
assign TP_PENIRQ_N   = GPIO_0[0];
assign TP_DOUT       = GPIO_0[1];
assign TP_BUSY       = GPIO_0[2];
assign GPIO_0[3]	 = TP_DIN;

assign GPIO_0[4]	 = LCD_TPn_sclk;

assign GPIO_0[35]    = LTM_SDA;
assign GPIO_0[34]    = LTM_SCEN;
assign GPIO_0[33]    = LTM_GRST;

assign GPIO_0[9]	 = LTM_NCLK;
assign GPIO_0[10]    = LTM_DEN;
assign GPIO_0[11]    = LTM_HD;
assign GPIO_0[12]    = LTM_VD;

assign GPIO_0[5]     = LTM_B[3];
assign GPIO_0[6]     = LTM_B[2];
assign GPIO_0[7]     = LTM_B[1];
assign GPIO_0[8]     = LTM_B[0];
assign GPIO_0[16:13] = LTM_B[7:4];
assign GPIO_0[24:17] = LTM_G[7:0];
assign GPIO_0[32:25] = LTM_R[7:0];

//trggg






always_ff @ (posedge CLOCK_50_I or negedge Resetn) begin
	if (~Resetn) begin
		three_sec_region_counter <= 28'd0;
	end else begin
			if(clear_3_sec_region_counter) begin
				three_sec_region_counter<=28'd0;
			end else if (TP_touch_en) begin
				three_sec_region_counter <= three_sec_region_counter + 28'd1;			
			end
	end
end



//Clock Division for 1 ms Clock
always_ff @ (posedge CLOCK_50_I or negedge Resetn) begin
	if (~Resetn) begin // || one_ms_reset_flag condition causing issues
		clock_1_ms_div_count <= 16'h0;
		//one_ms_count<=14'd0;

	end else begin
		if (clock_1_ms_div_count < 'd24999) begin
			clock_1_ms_div_count <= clock_1_ms_div_count + 16'd1;
		end else begin
			clock_1_ms_div_count <= 16'h0;
			//one_ms_count<=one_ms_count + 14'd1;
		end
	end
end

always_ff @ (posedge CLOCK_50_I or negedge Resetn) begin
//logic [15:0] clock_1_ms_div_count;
//logic clock_1_ms, clock_1_ms_buf;
	if (Resetn == 1'b0) begin
		clock_1_ms <= 1'b1;
	end else begin
		if (clock_1_ms_div_count == 'd0) clock_1_ms <= ~clock_1_ms;
	end
end

always_ff @ (posedge CLOCK_50_I or negedge Resetn) begin
	if (Resetn == 1'b0) begin
		clock_1_ms_buf <= 1'b1;	
	end else begin
		clock_1_ms_buf <= clock_1_ms;
	end
end



// Top state machine for controlling resets
always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) begin
		Top_state <= 3'h0;
		TP_enable <= 1'b0;
		LCD_enable <= 1'b0;
		LCD_config_start <= 1'b0;
		LCD_TPn_sel <= 1'b1;
	end else begin
		case (Top_state)
			3'h0 : begin
				LCD_config_start <= 1'b1;
				LCD_TPn_sel <= 1'b1;
				Top_state <= 3'h1;
			end			
			3'h1 : begin
				LCD_config_start <= 1'b0;
				if (LCD_config_done & ~LCD_config_start) begin
					TP_enable <= 1'b1;
					LCD_enable <= 1'b1;
					LCD_TPn_sel <= 1'b0;
					Top_state <= 3'h2;
				end
			end			
			3'h2 : begin
				Top_state <= 3'h2;
			end
		endcase
	end
end				

// LCD Configuration
LCD_Config_Controller LCD_Config_unit(
	.Clock(Clock),
	.Resetn(Resetn),
	.Start(LCD_config_start),
	.Done(LCD_config_done),
	.LCD_I2C_sclk(LTM_SCLK),
 	.LCD_I2C_sdat(LTM_SDA),
	.LCD_I2C_scen()
);

// LCD Image
LCD_Data_Controller LCD_Data_unit (
	.Clock(Clock),
	.oClock_en(),
	.Resetn(Resetn),
	.Enable(LCD_enable),
	.iRed(Colourbar_Red),
	.iGreen(Colourbar_Green),
	.iBlue(Colourbar_Blue),
	.oCoord_X(Colourbar_X),
	.oCoord_Y(Colourbar_Y),
	.H_Count(), // not used in this experiment
	.V_Count(), // not used in this experiment
	.LTM_NCLK(LTM_NCLK),
	.LTM_HD(LTM_HD),
	.LTM_VD(LTM_VD),
	.LTM_DEN(LTM_DEN),
	.LTM_R(LTM_R),
	.LTM_G(LTM_G),
	.LTM_B(LTM_B)
);
////////////////////////////////////////////////////
// State machine for generating the colour bars
////////////////////////////////////////////////////
always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) begin
		Colourbar_Red <= 8'h00; 
		Colourbar_Green <= 8'h00;
		Colourbar_Blue <= 8'h00;
	end else begin
		//seg 0
		if(Colourbar_X<'d200 && Colourbar_Y<'d240) begin
			Colourbar_Red <= {8{block_colour_0[2]}};
			Colourbar_Green<= {8{block_colour_0[1]}};
			Colourbar_Blue<= {8{block_colour_0[0]}};
		end
		// seg 1
		else if((Colourbar_X>'d199 && Colourbar_X < 'd400)&& Colourbar_Y<'d240) begin
			Colourbar_Red <= {8{block_colour_1[2]}};
			Colourbar_Green<= {8{block_colour_1[1]}};
			Colourbar_Blue<= {8{block_colour_1[0]}};
		end
		// seg 2
		else if((Colourbar_X>'d399 && Colourbar_X < 'd600) && Colourbar_Y<'d240) begin
			Colourbar_Red <= {8{block_colour_2[2]}};
			Colourbar_Green<= {8{block_colour_2[1]}};
			Colourbar_Blue<= {8{block_colour_2[0]}};
		end
		// seg 3
		else if((Colourbar_X>'d599 && Colourbar_X < 'd800) && Colourbar_Y<'d240) begin
			Colourbar_Red <= {8{block_colour_3[2]}};
			Colourbar_Green<= {8{block_colour_3[1]}};
			Colourbar_Blue<= {8{block_colour_3[0]}};
		end
		// seg 4
		else if(Colourbar_X<'d200 && Colourbar_Y>'d239) begin
			Colourbar_Red <= {8{block_colour_4[2]}};
			Colourbar_Green<= {8{block_colour_4[1]}};
			Colourbar_Blue<= {8{block_colour_4[0]}};
		end
		// seg 5
		else if((Colourbar_X>'d199 && Colourbar_X < 'd400) && Colourbar_Y>'d239) begin
			Colourbar_Red <= {8{block_colour_5[2]}};
			Colourbar_Green<= {8{block_colour_5[1]}};
			Colourbar_Blue<= {8{block_colour_5[0]}};
		end
		// seg 6
		else if((Colourbar_X>'d399 && Colourbar_X < 'd600) && Colourbar_Y>'d239) begin
			Colourbar_Red <= {8{block_colour_6[2]}};
			Colourbar_Green<= {8{block_colour_6[1]}};
			Colourbar_Blue<= {8{block_colour_6[0]}};
		end
		// seg 7
		else if((Colourbar_X>'d599 && Colourbar_X < 'd800) && Colourbar_Y>'d239) begin
			Colourbar_Red <= {8{block_colour_7[2]}};
			Colourbar_Green<={8{block_colour_7[1]}};
			Colourbar_Blue<= {8{block_colour_7[0]}};
		end


	end
end

// Controller for the TP on the LTM
Touch_Panel_Controller Touch_Panel_unit(
	.Clock_50MHz(Clock),
	.Resetn(Resetn),
	.Enable(~LTM_VD),	
	.Touch_En(TP_touch_en),
	.Coord_En(TP_coord_en),
	.X_Coord(TP_X_coord),
	.Y_Coord(TP_Y_coord),
	.TP_PENIRQ_N_I(TP_PENIRQ_N),
	.TP_BUSY_I(TP_BUSY),
	.TP_SCLK_O(TP_DCLK),
	.TP_MOSI_O(TP_DIN),
	.TP_MISO_I(TP_DOUT),
	.TP_SS_N_O(TP_CS)
);


////////////////////////////////////////////////////////////////////////////////////////////////////////
// State machine for capturing the touch panel coordinates
////////////////////////////////////////////////////////////////////////////////////////////////////////
always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) begin
		block_colour_0<=3'b000;
		block_colour_1<=3'b001;
		block_colour_2<=3'b010;
		block_colour_3<=3'b011;
		block_colour_4<=3'b100;
		block_colour_5<=3'b101;
		block_colour_6<=3'b110;
		block_colour_7<=3'b111;
		
		block_colour_0_buf<=3'b000;
		block_colour_1_buf<=3'b001;
		block_colour_2_buf<=3'b010;
		block_colour_3_buf<=3'b011;
		block_colour_4_buf<=3'b100;
		block_colour_5_buf<=3'b101;
		block_colour_6_buf<=3'b110;
		block_colour_7_buf<=3'b111;
		
		TP_touch_en_buf <= 1'b0;
		TP_coord_en_buf <= 1'b0;
		
		//logic [4:0] BCD_count[3:0];
		BCD_count[0] <= 4'h0;
		BCD_count[1] <= 4'h0;
		BCD_count[2] <= 4'h0;
		BCD_count[3] <= 4'h0;
		
		block_index<=4'b1000;  //make sure these two index are initialized the same
		block_index_buf<=4'b1000;  //and greater than 'd7
		
		VSYNC_UPDATE <= 2'b0;
		clear_3_sec_region_counter <= 1'b0;

	end else begin
		TP_touch_en_buf <= TP_touch_en;
		TP_coord_en_buf <= TP_coord_en;
		clear_3_sec_region_counter <= 1'b0; //make sure to set 3 sec region counter to count, only reset counter under certain condition
		
		VSYNC_UPDATE <= {VSYNC_UPDATE[0], LTM_VD};
				
		sampleFlag_buf<=sampleFlag;		
		if(TP_touch_en && ~TP_touch_en_buf && ~TP_coord_en)begin
			sampleFlag<=1'b1;
		end
		
		if(TP_coord_en && ~TP_coord_en_buf && TP_touch_en)begin
			sampleFlag<=1'b0;
		end
		
		if(TP_coord_en && (~TP_touch_en && TP_touch_en_buf)) begin
			if(block_index!=block_index_buf) begin  //dragged across region
				clear_3_sec_region_counter <= 1'b1; //reset the 3 sec region counter
			end
//089879078
		end
		
		if(~TP_touch_en && TP_touch_en_buf) begin //TP is let go of
			clear_3_sec_region_counter <= 1'b1; //reset the 3 sec region counter since	
		end
				
		//////
		//this if block is for incrementing current and previous block region after new touch event ONLY (3 sec continous touch case handled below!!!!)
		if(~sampleFlag && sampleFlag_buf) begin 
		//~sampleFlag && sampleFlag_buf || (((block_index==block_index_buf)&&(TP_touch_en)&& TP_coord_en))
				
				//compute current region
		
				//seg 0 /region 000
				if (TP_X_coord<'d1024 && TP_Y_coord<'d2048) begin
					block_colour_0_buf <= block_colour_0_buf +3'd1;
				end
				//seg 1 /region 001
				else if (TP_X_coord>'d1023 && TP_X_coord<'d2048 && TP_Y_coord<'d2048) begin
					block_colour_1_buf <= block_colour_1_buf +3'd1;
				end
				//seg 2 /region 010
				else if (TP_X_coord>'d2047 && TP_X_coord<'d3072 && TP_Y_coord<'d2048) begin
					block_colour_2_buf <= block_colour_2_buf +3'd1;
				end
				//seg 3 /region 011
				else if (TP_X_coord>'d3071 && TP_X_coord<'d4096 && TP_Y_coord<'d2048) begin
					block_colour_3_buf <= block_colour_3_buf +3'd1;
				end
				//seg 4 /region 100
				else if (TP_X_coord<'d1024 && TP_Y_coord>'d2047) begin
					block_colour_4_buf <= block_colour_4_buf +3'd1;
				end
				//seg 5 /region 101
				else if (TP_X_coord>'d1023 && TP_X_coord<'d2048 && TP_Y_coord>'d2047) begin
					block_colour_5_buf <= block_colour_5_buf +3'd1;

				end
				//seg 6 /region 110
				else if (TP_X_coord>'d2047 && TP_X_coord<'d3072 && TP_Y_coord>'d2047) begin
					block_colour_6_buf <= block_colour_6_buf +3'd1;

				end
				//seg 7 /region 111
				else if (TP_X_coord>'d3071 && TP_X_coord<'d4096 && TP_Y_coord>'d2047) begin
					block_colour_7_buf <= block_colour_7_buf +3'd1;
				end
		end
		
		
		//this block of code creates a record of prev and current touched regions
		//Details:
		//if touch is detected, then store record of the previous clock cycle's touched region
		//and current clock cycle's touched region
		if(TP_coord_en) begin
				
				//seg 0 /region 000
				if (TP_X_coord<'d1024 && TP_Y_coord<'d2048) begin
					block_index<=4'b0000;
					block_index_buf<=block_index;
				end
				//seg 1 /region 001
				else if (TP_X_coord>'d1023 && TP_X_coord<'d2048 && TP_Y_coord<'d2048) begin
					block_index<=4'b0001;
					block_index_buf<=block_index;
				end
				//seg 2 /region 010
				else if (TP_X_coord>'d2047 && TP_X_coord<'d3072 && TP_Y_coord<'d2048) begin
					block_index<=4'b0010;
					block_index_buf<=block_index;
				end
				//seg 3 /region 011
				else if (TP_X_coord>'d3071 && TP_X_coord<'d4096 && TP_Y_coord<'d2048) begin
					block_index<=4'b0011;
					block_index_buf<=block_index;
				end
				//seg 4 /region 100
				else if (TP_X_coord<'d1024 && TP_Y_coord>'d2047) begin
					block_index<=4'b0100;
					block_index_buf<=block_index;
				end
				//seg 5 /region 101
				else if (TP_X_coord>'d1023 && TP_X_coord<'d2048 && TP_Y_coord>'d2047) begin
					block_index<=4'b0101;
					block_index_buf<=block_index;
				end
				//seg 6 /region 110
				else if (TP_X_coord>'d2047 && TP_X_coord<'d3072 && TP_Y_coord>'d2047) begin
					block_index<=4'b0110;
					block_index_buf<=block_index;
				end
				//seg 7 /region 111
				else if (TP_X_coord>'d3071 && TP_X_coord<'d4096 && TP_Y_coord>'d2047) begin
					//block_colour_7_buf <= block_colour_7_buf +3'd1;
					block_index<=4'b0111;
					block_index_buf<=block_index;
				end
		end
		//if hit 3 seconds of being in the same region
		if(three_sec_region_counter == 28'd149_999_999) begin
			//do a set of case statements based of current block index
			if(block_index=='d0) begin //black
				block_colour_0_buf <= block_colour_0_buf +3'd1;
			end else if(block_index=='d1) begin
				block_colour_1_buf <= block_colour_1_buf +3'd1;
			end else if(block_index=='d2) begin
				block_colour_2_buf <= block_colour_2_buf +3'd1;
			end else if(block_index=='d3) begin
				block_colour_3_buf <= block_colour_3_buf +3'd1;
			end else if(block_index=='d4) begin
				block_colour_4_buf <= block_colour_4_buf +3'd1;
			end else if(block_index=='d5) begin
				block_colour_5_buf <= block_colour_5_buf +3'd1;
			end else if(block_index=='d6) begin
				block_colour_6_buf <= block_colour_6_buf +3'd1;
			end else begin //white  if(block_index=='d7) 
				block_colour_7_buf <= block_colour_7_buf +3'd1;
			end
			clear_3_sec_region_counter <= 1'b1; //reset the 3 sec region counter to start counting for next 3 sec	
		end
	//	&& TP_touch_en
		//update actual block colour at VSYNC edge //////fixes half block line thingy
		if(VSYNC_UPDATE == 2'b10 ) begin
			block_colour_0 <= block_colour_0_buf;
			block_colour_1 <= block_colour_1_buf;
			block_colour_2 <= block_colour_2_buf;
			block_colour_3 <=	block_colour_3_buf;
			block_colour_4 <= block_colour_4_buf;
			block_colour_5 <= block_colour_5_buf;
			block_colour_6 <= block_colour_6_buf;
			block_colour_7 <= block_colour_7_buf;
		end
		
		//bcd counter controlled here increment when touched and on 1ms clock edges
		if(TP_touch_en && clock_1_ms && ~clock_1_ms_buf) begin 
		//must increment lsb every one millisecond
			if(BCD_count[0] < 4'd9) begin 
				BCD_count[0] <= BCD_count[0] + 4'h1;
			end else begin	
				BCD_count[0]<=4'h0;
				if(BCD_count[1]<4'd9) begin
					BCD_count[1]<=BCD_count[1]+4'd1;
				end else begin	
					BCD_count[1]<=4'h0;
					if(BCD_count[2]<4'd9) begin 
						BCD_count[2]<=BCD_count[2]+4'd1;
					end else begin
						BCD_count[2]<=4'h0;
						if(BCD_count[3]<4'd9) begin
							BCD_count[3]<=BCD_count[3]+4'd1;
								end else BCD_count[3]<=4'h0;
					end
				end
			end	
		end	
		//reset bcd if touch panel let go of
		if(~TP_touch_en && TP_touch_en_buf)begin
			BCD_count[0]<=4'h0;
			BCD_count[1]<=4'h0;
			BCD_count[2]<=4'h0;
			BCD_count[3]<=4'h0;
		end
		//reset bcd if moved regions
		if(block_index != block_index_buf)begin
			BCD_count[0]<=4'h0;
			BCD_count[1]<=4'h0;
			BCD_count[2]<=4'h0;
			BCD_count[3]<=4'h0;
		end
	end
end

logic [7:0] amt_black;
logic [7:0] amt_blue;
logic [7:0] amt_green;
logic [7:0] amt_turquoise;
logic [7:0] amt_red;
logic [7:0] amt_magenta;
logic [7:0] amt_yellow;
logic [7:0] amt_white;

logic [6:0] black;
logic [6:0] blue;
logic [6:0] green;
logic [6:0] turquoise;
logic [6:0] red;
logic [6:0] magenta;
logic [6:0] yellow;
logic [6:0] white;

logic [3:0] sum_black;
logic [3:0] sum_blue;
logic [3:0] sum_green;
logic [3:0] sum_turquoise;
logic [3:0] sum_red;
logic [3:0] sum_magenta;
logic [3:0] sum_yellow;
logic [3:0] sum_white;

logic [7:0] max;
logic [7:0] second_max;

//flag detects if all 8 regions same color
logic allSameColourFlag;

always_comb begin
//block_colour_7
	
	max = 8'd0000000;
	second_max = 8'd00000000;
	amt_black={8{1'b0}};
	amt_blue={8{1'b0}};
	amt_green={8{1'b0}};
	amt_turquoise={8{1'b0}};
	amt_red={8{1'b0}};
	amt_magenta={8{1'b0}};
	amt_yellow={8{1'b0}};
	amt_white={8{1'b0}};
	allSameColourFlag=1'b0;

	
	sum_black=4'b0;
	sum_blue=4'b0;
	sum_green=4'b0;
	sum_turquoise=4'b0;
	sum_red=4'b0;
	sum_magenta=4'b0;
	sum_yellow=4'b0;
	sum_white=4'b0;
	
	//count amount of blacks aka    000 | 0

	
	if(block_colour_0==3'b000)amt_black[7]=1'b1;
	if(block_colour_1==3'b000)amt_black[6]=1'b1;
	if(block_colour_2==3'b000)amt_black[5]=1'b1;
	if(block_colour_3==3'b000)amt_black[4]=1'b1;
	if(block_colour_4==3'b000)amt_black[3]=1'b1;
	if(block_colour_5==3'b000)amt_black[2]=1'b1;
	if(block_colour_6==3'b000)amt_black[1]=1'b1;
	if(block_colour_7==3'b000)amt_black[0]=1'b1;
	
	if(block_colour_0==3'b001)amt_blue[7]=1'b1;
	if(block_colour_1==3'b001)amt_blue[6]=1'b1;
	if(block_colour_2==3'b001)amt_blue[5]=1'b1;
	if(block_colour_3==3'b001)amt_blue[4]=1'b1;
	if(block_colour_4==3'b001)amt_blue[3]=1'b1;
	if(block_colour_5==3'b001)amt_blue[2]=1'b1;
	if(block_colour_6==3'b001)amt_blue[1]=1'b1;
	if(block_colour_7==3'b001)amt_blue[0]=1'b1;
	
	if(block_colour_0==3'b010)amt_green[7]=1'b1;
	if(block_colour_1==3'b010)amt_green[6]=1'b1;
	if(block_colour_2==3'b010)amt_green[5]=1'b1;
	if(block_colour_3==3'b010)amt_green[4]=1'b1;
	if(block_colour_4==3'b010)amt_green[3]=1'b1;
	if(block_colour_5==3'b010)amt_green[2]=1'b1;
	if(block_colour_6==3'b010)amt_green[1]=1'b1;
	if(block_colour_7==3'b010)amt_green[0]=1'b1;
	
	if(block_colour_0==3'b011)amt_turquoise[7]=1'b1;
	if(block_colour_1==3'b011)amt_turquoise[6]=1'b1;
	if(block_colour_2==3'b011)amt_turquoise[5]=1'b1;
	if(block_colour_3==3'b011)amt_turquoise[4]=1'b1;
	if(block_colour_4==3'b011)amt_turquoise[3]=1'b1;
	if(block_colour_5==3'b011)amt_turquoise[2]=1'b1;
	if(block_colour_6==3'b011)amt_turquoise[1]=1'b1;
	if(block_colour_7==3'b011)amt_turquoise[0]=1'b1;
	
	if(block_colour_0==3'b100)amt_red[7]=1'b1;
	if(block_colour_1==3'b100)amt_red[6]=1'b1;
	if(block_colour_2==3'b100)amt_red[5]=1'b1;
	if(block_colour_3==3'b100)amt_red[4]=1'b1;
	if(block_colour_4==3'b100)amt_red[3]=1'b1;
	if(block_colour_5==3'b100)amt_red[2]=1'b1;
	if(block_colour_6==3'b100)amt_red[1]=1'b1;
	if(block_colour_7==3'b100)amt_red[0]=1'b1;
	
	if(block_colour_0==3'b101)amt_magenta[7]=1'b1;
	if(block_colour_1==3'b101)amt_magenta[6]=1'b1;
	if(block_colour_2==3'b101)amt_magenta[5]=1'b1;
	if(block_colour_3==3'b101)amt_magenta[4]=1'b1;
	if(block_colour_4==3'b101)amt_magenta[3]=1'b1;
	if(block_colour_5==3'b101)amt_magenta[2]=1'b1;
	if(block_colour_6==3'b101)amt_magenta[1]=1'b1;
	if(block_colour_7==3'b101)amt_magenta[0]=1'b1;
	
	
	if(block_colour_0==3'b110)amt_yellow[7]=1'b1;
	if(block_colour_1==3'b110)amt_yellow[6]=1'b1;
	if(block_colour_2==3'b110)amt_yellow[5]=1'b1;
	if(block_colour_3==3'b110)amt_yellow[4]=1'b1;
	if(block_colour_4==3'b110)amt_yellow[3]=1'b1;
	if(block_colour_5==3'b110)amt_yellow[2]=1'b1;
	if(block_colour_6==3'b110)amt_yellow[1]=1'b1;
	if(block_colour_7==3'b110)amt_yellow[0]=1'b1;
	
	if(block_colour_0==3'b111)amt_white[7]=1'b1;
	if(block_colour_1==3'b111)amt_white[6]=1'b1;
	if(block_colour_2==3'b111)amt_white[5]=1'b1;
	if(block_colour_3==3'b111)amt_white[4]=1'b1;
	if(block_colour_4==3'b111)amt_white[3]=1'b1;
	if(block_colour_5==3'b111)amt_white[2]=1'b1;
	if(block_colour_6==3'b111)amt_white[1]=1'b1;
	if(block_colour_7==3'b111)amt_white[0]=1'b1;

	
	sum_black=amt_black[7]+amt_black[6]+amt_black[5]+amt_black[4]+amt_black[3]+amt_black[2]+amt_black[1]+amt_black[0];
	sum_blue=amt_blue[7]+amt_blue[6]+amt_blue[5]+amt_blue[4]+amt_blue[3]+amt_blue[2]+amt_blue[1]+amt_blue[0];
	sum_green=amt_green[7]+amt_green[6]+amt_green[5]+amt_green[4]+amt_green[3]+amt_green[2]+amt_green[1]+amt_green[0];
	sum_turquoise=amt_turquoise[7]+amt_turquoise[6]+amt_turquoise[5]+amt_turquoise[4]+amt_turquoise[3]+amt_turquoise[2]+amt_turquoise[1]+amt_turquoise[0];
	sum_red=amt_red[7]+amt_red[6]+amt_red[5]+amt_red[4]+amt_red[3]+amt_red[2]+amt_red[1]+amt_red[0];
	sum_magenta=amt_magenta[7]+amt_magenta[6]+amt_magenta[5]+amt_magenta[4]+amt_magenta[3]+amt_magenta[2]+amt_magenta[1]+amt_magenta[0];
	sum_yellow=amt_yellow[7]+amt_yellow[6]+amt_yellow[5]+amt_yellow[4]+amt_yellow[3]+amt_yellow[2]+amt_yellow[1]+amt_yellow[0];
	sum_white=amt_white[7]+amt_white[6]+amt_white[5]+amt_white[4]+amt_white[3]+amt_white[2]+amt_white[1]+amt_white[0];
	
	
	black={3'b000,sum_black}; //3+1+4 = 8 bits
	blue={3'b001,sum_blue};
	green={3'b010,sum_green};
	turquoise={3'b011,sum_turquoise};
	red={3'b100,sum_red};
	magenta={3'b101,sum_magenta};
	yellow={3'b110,sum_yellow};
	white={3'b111,sum_white};

	//count amount of black aka     000 | 0
	//count amount of blue aka      001 | 1
	//count amount of green aka     010 | 2
	//count amount of turquoise aka 011 | 3
	//count amount of red aka       100 | 4
	//count amount of magenta aka   101 | 5
	//count amount of yellow aka    110 | 6
	//count amount of white aka     111 | 7

	//priority encoder to find max based on colour priority
	if(sum_black >= max[3:0]) begin
		max = black;
	end
	if(sum_blue >= max[3:0]) begin
		max = blue;
	end
	if(sum_green >= max[3:0]) begin
		max = green;
	end
	if(sum_turquoise >= max[3:0]) begin
		max = turquoise;
	end
	if(sum_red >= max[3:0]) begin
		max = red;
	end
	if(sum_magenta >= max[3:0]) begin
		max = magenta;
	end
	if(sum_yellow >= max[3:0]) begin
		max = yellow;
	end
	if(sum_white >= max[3:0]) begin
		max = white;
	end
	
	//priority encoder to find second max based on colour priority
	if(max != 'd0 && max != black && sum_black >= second_max[3:0]) begin
		second_max = black;
	end
	if(max != 'd0 && max != blue && sum_blue >= second_max[3:0]) begin
		second_max = blue;
	end
	if(max != 'd0 && max != green && sum_green >= second_max[3:0]) begin
		second_max = green;
	end
	if(max != 'd0 && max != turquoise && sum_turquoise >= second_max[3:0]) begin
		second_max = turquoise;
	end
	if(max != 'd0 && max != red && sum_red >= second_max[3:0]) begin
		second_max = red;
	end
	if(max != 'd0 && max != magenta && sum_magenta >= second_max[3:0]) begin
		second_max = magenta;
	end
	if(max != 'd0 && max != yellow && sum_yellow >= second_max[3:0]) begin
		second_max = yellow;
	end
	if(max != 'd0 && max != white && sum_white >= second_max[3:0]) begin
		second_max = white;
	end
	
	if ((sum_black==4'd8)||(sum_blue==4'd8)||(sum_green==4'd8)||(sum_turquoise==4'd8)||(sum_red==4'd8)||(sum_magenta==4'd8)||(sum_yellow==4'd8)||(sum_white==4'd8))begin
		allSameColourFlag=1'b1;
	end else allSameColourFlag=1'b0;

end

//logic 	[4:0] 	touchDuration[7:0];
assign touchDuration[0][4:0] = {TP_touch_en,BCD_count[0]};
assign touchDuration[1][4:0] = {TP_touch_en,BCD_count[1]};
assign touchDuration[2][4:0] = {TP_touch_en,BCD_count[2]};
assign touchDuration[3][4:0] = {TP_touch_en,BCD_count[3]};

assign touchDuration[4][4:0] = {~allSameColourFlag,second_max[3:0]}; // frequency
assign touchDuration[5][4:0] = {~allSameColourFlag,1'b0,second_max[6:4]}; //colour code

//works
assign touchDuration[6][4:0] = {1'b1,max[3:0]}; // frequency
assign touchDuration[7][4:0] = {1'b1,1'b0,max[6:4]}; //colour code  //max = {color_code,1'b0,sum_colour}; //3+1+4 = 8 bits

seven_seg_displays display_unit (
	.hex_values(touchDuration),
	.SEVEN_SEGMENT_N_O(SEVEN_SEGMENT_N_O)
);

endmodule
