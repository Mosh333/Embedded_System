// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

`timescale 1ns/100ps
`default_nettype none

// This module implements the image filter pipe
module Filter_Pipe (
	input logic Clock,
	input logic Clock_en,
	input logic Resetn,

	input logic Enable,
	input logic [31:0] Filter_config,

	input logic [10:0] H_Count,
	input logic [9:0] V_Count,

	output logic oRead_in_en,
	input logic [7:0] R_in,
	input logic [7:0] G_in,
	input logic [7:0] B_in,
	
	input logic iRead_out_en,
	output logic [7:0] R_out,
	output logic [7:0] G_out,
	output logic [7:0] B_out,
	input logic [1:0] vsync_edge
);

always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) oRead_in_en <= 1'b0;
	else if (~Enable) oRead_in_en <= 1'b0;
	else oRead_in_en <= (
		(H_Count > (11'd216 - 11'd2)) &&
		(H_Count < (11'd216 - 11'd2 + 11'd640 + 11'd1)) &&
		(V_Count > (10'd35 - 10'd1 - 10'd1 - 10'd1)) &&
		(V_Count < (10'd35 - 10'd1 + 10'd480 + 10'd1 - 10'd1 - 10'd1))
	) ? ~Clock_en : 1'b0;
end

logic [7:0] Red_rddata_0b, Green_rddata_0b, Blue_rddata_0b;
logic [7:0] Red_wrdata_1a, Green_wrdata_1a, Blue_wrdata_1a;

logic DP_wren_0a;
logic [9:0] DP_addr_0a, DP_addr_0b;

logic [25:0] one_sec_counter;
logic [1:0] VSYNC_UPDATE;

logic [25:0] clock_1_s_div_count;
logic start_1_sec_counter;
logic clock_1_s;
logic clock_1_s_buf;
logic update_filter_flag;

//////////////////////////////
//1 sec clock wave generate
//////////////////////////////
//Clock Division for 1 second counter
//always_ff @ (posedge Clock or negedge Resetn) begin
//	if (~Resetn) begin
//		one_sec_counter <= 26'd0;
//	end else begin
//		//count to 'd24999 for one_sec_counter
//		if (clear_1_sec_counter==1'b1) begin
//			one_sec_counter <= 26'd0;
//		end else begin
//			one_sec_counter <= one_sec_counter + 26'd1;
//		end
//	end
//end

/////////////////////////////////////////////////////////////////////

//Clock Division for 1 sec Clock
always_ff @ (posedge Clock or negedge Resetn) begin
	if (~Resetn) begin // || one_ms_reset_flag condition causing issues
		clock_1_s_div_count <= 26'h0;
		//one_ms_count<=14'd0;

	end else begin
		if ((clock_1_s_div_count < 'd24999999) && start_1_sec_counter) begin
			clock_1_s_div_count <= clock_1_s_div_count + 26'd1;
		end else begin
			clock_1_s_div_count <= 26'h0;
			//one_ms_count<=one_ms_count + 14'd1;
		end
	end
end

always_ff @ (posedge Clock or negedge Resetn) begin
	if (Resetn == 1'b0) begin
		clock_1_s <= 1'b1;
	end else begin
		if (clock_1_s_div_count == 'd0) clock_1_s <= ~clock_1_s;
	end
end

always_ff @ (posedge Clock or negedge Resetn) begin
	if (Resetn == 1'b0) begin
		clock_1_s_buf <= 1'b1;	
	end else begin
		clock_1_s_buf <= clock_1_s;
	end
end



////////////////////////////////////////////////////////////////

always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) begin
		DP_wren_0a <= 1'b0;
		DP_addr_0a <= 10'd0;
	end else if (~Enable) begin
		DP_wren_0a <= 1'b0;
		DP_addr_0a <= 10'd0;		
	end else if (Clock_en) begin
		DP_wren_0a <= oRead_in_en;
		if (DP_wren_0a) DP_addr_0a <= DP_addr_0a + 10'd1;
	end
end




Filter_RAM Filter_RAM_R0 (
	.address_a(DP_addr_0a),
	.address_b(DP_addr_0b),
	.clock(Clock),
	.data_a(R_in),
	.data_b(8'h00),
	.wren_a(DP_wren_0a),
	.wren_b(1'b0),
	.q_a(),
	.q_b(Red_rddata_0b)
);

Filter_RAM Filter_RAM_G0 (
	.address_a(DP_addr_0a),
	.address_b(DP_addr_0b),
	.clock(Clock),
	.data_a(G_in),
	.data_b(8'h00),
	.wren_a(DP_wren_0a),
	.wren_b(1'b0),
	.q_a(),
	.q_b(Green_rddata_0b)
);

Filter_RAM Filter_RAM_B0 (
	.address_a(DP_addr_0a),
	.address_b(DP_addr_0b),
	.clock(Clock),
	.data_a(B_in),
	.data_b(8'h00),
	.wren_a(DP_wren_0a),
	.wren_b(1'b0),
	.q_a(),
	.q_b(Blue_rddata_0b)
);

logic Read_0_en;
logic [9:0] DP_addr_0b_reg;

logic DP_wren_1a;
logic [9:0] DP_addr_1a, DP_addr_1b;

logic [7:0] Y_calc, Red_calc, Green_calc, Blue_calc;
logic [21:0] Y_calc_long;

assign Y_calc_long = 
	(22'd1052 * Red_rddata_0b) + 
	(22'd2064 * Green_rddata_0b) + 
	(22'd401 * Blue_rddata_0b);
assign Y_calc = Y_calc_long[19:12];

logic [7:0] Y_m1, Y_0, Y_p1;
logic [4:0] filter_en;

logic [11:0] Filter_calc_1;
logic [7:0] Filter_calc_2;
logic [9:0] Temp;
logic [7:0] Filter_calc_3;
logic [7:0] Filter_calc_4;

logic [7:0] Y_m2, Y_p2;
logic [11:0] Temp2;
logic [9:0] three_Y_m1;
logic [10:0] six_Y_0;
logic [9:0] three_Y_p1;

logic [2:0] caseCounter;
logic [2:0] caseCounter_buf;
logic case7flag;


assign Filter_calc_1 = {4'h0, Y_0} + {4'h0, Y_p1};
assign Filter_calc_2 = Filter_calc_1[8:1];
//implements Y[i] = (Y[i-1] + 2*Y[i] + Y[i+1])/4
assign Temp = {2'd0,Y_m1} + {1'd0,Y_0,1'd0} + {2'd0,Y_p1};
assign Filter_calc_3 = Temp[9:2];

//3x2^8 is 768 requires 10 bits
//6x2^8 is 1536 requires 11 bits
always_comb begin
	three_Y_m1 = 'd3*Y_m1;
	six_Y_0 = 'd6*Y_0;
	three_Y_p1 = 'd3*Y_p1;
	//implements Y[i] = (2*Y[i-2] + 3*Y[i-1] + 6*Y[i] + 3*Y[i+1] + 2*Y[i+2])/16
	Temp2 = {3'd0,Y_m2,1'd0} + {2'd0,three_Y_m1} + {1'd0,six_Y_0} + {2'd0, three_Y_p1} + {3'd0,Y_p2,1'd0}; 
	Filter_calc_4 = Temp2[11:4];
end

always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) begin
		DP_addr_0b <= 10'h000;
		filter_en <= 5'b00000;
		DP_wren_1a <= 1'b0;
	end else if (~Enable) begin
		DP_addr_0b <= 10'h000;
		filter_en <= 5'b00000;
		DP_wren_1a <= 1'b0;
	end else if (Clock_en) begin
		DP_addr_0b <= DP_addr_0a;
		filter_en <= {filter_en[3:0],DP_wren_0a};//msb shift in
		
		if(Filter_config[2:0] == 3'd6 || (Filter_config[2:0] == 3'd7 && caseCounter == 3'd6)) begin  //uses Ym2 Ym1 Y0 Yp1 Yp2
			//to implement 5-Tap filter, need en_4?
			DP_wren_1a <= filter_en[4]; //assign write enable to be the slowest filter enable term
		end else if(Filter_config[2:0] == 3'd5) begin //uses Ym1 Y0 Yp1
			DP_wren_1a <= filter_en[3];
		end else if (Filter_config[2:0] == 3'd4) begin //uses Ym1 Y0 Yp1
			//for three tap filter enable
			DP_wren_1a <= filter_en[3];
		end else DP_wren_1a <= filter_en[1];
	end
end
//ll
//
always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) begin
		Y_m2	<= 8'd0;
		Y_m1 <= 8'd0;
		Y_0 <= 8'd0;
		Y_p1 <= 8'd0;
		Y_p2<= 8'd0;
	end else if (Clock_en) begin
		if(Filter_config[2:0]<3'd6) begin
			//Averaging values between 
			Y_0 <= Y_p1;		
			// for lead-in corner case
			if (filter_en[2] & ~filter_en[3]) 
				Y_m1 <= Y_p1;
			else Y_m1 <= Y_0;
			// for take-down corner case
			if (filter_en[1]) 
				Y_p1 <= Y_calc;
				
		end else if(Filter_config[2:0]==3'd6 || (Filter_config[2:0] == 3'd7 && caseCounter == 3'd6)) begin
			//Averaging values between 
			Y_p1 <= Y_p2;
			Y_0 <= Y_p1;
			// for lead-in corner case
			if (filter_en[3] & ~filter_en[4]) begin //filter_en[2] & ~filter_en[3]
				Y_m1 <= Y_p1;  //was Y_p2
				Y_m2 <= Y_p1;  //was Y_p2
			end else begin  //do this if not in lead-in to handle common case
				Y_m1 <= Y_0;  //was Y_0
				Y_m2 <= Y_m1;  //was Y_m1
			end
			// for take-down corner case
			if (filter_en[1]) 
				Y_p2 <= Y_calc;
		end

	end
end

always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) begin		
		Red_wrdata_1a <= 8'h00;
		Green_wrdata_1a <= 8'h00;
		Blue_wrdata_1a <= 8'h00;
		start_1_sec_counter <= 1'b0;
		caseCounter<=3'd0;
		caseCounter_buf<=3'd0;
		update_filter_flag<=1'b0;
		case7flag<=1'b0;
	end else if (Clock_en) begin

		case (Filter_config[2:0])
			3'd0 : begin
				Red_wrdata_1a <= Red_rddata_0b;
				Green_wrdata_1a <= Green_rddata_0b;
				Blue_wrdata_1a <= Blue_rddata_0b;
				start_1_sec_counter <= 1'b0;
			end
			3'd1 : begin
				Red_wrdata_1a <= ~Red_rddata_0b;
				Green_wrdata_1a <= ~Green_rddata_0b;
				Blue_wrdata_1a <= ~Blue_rddata_0b;
				start_1_sec_counter <= 1'b0;
			end
			3'd2 : begin
				Red_wrdata_1a <= Y_calc;
				Green_wrdata_1a <= Y_calc;
				Blue_wrdata_1a <= Y_calc;
				start_1_sec_counter <= 1'b0;
			end
			3'd3 : begin
				Red_wrdata_1a <= ~Y_calc;
				Green_wrdata_1a <= ~Y_calc;
				Blue_wrdata_1a <= ~Y_calc;
				start_1_sec_counter <= 1'b0;
			end
			3'd4 : begin
				Red_wrdata_1a <= Filter_calc_2;
				Green_wrdata_1a <= Filter_calc_2;
				Blue_wrdata_1a <= Filter_calc_2;
				start_1_sec_counter <= 1'b0;
			end
			3'd5 : begin
				Red_wrdata_1a <= Filter_calc_3;
				Green_wrdata_1a <= Filter_calc_3;
				Blue_wrdata_1a <= Filter_calc_3;
				start_1_sec_counter <= 1'b0;
			end
			// FIVE TAP FILTER
			3'd6 : begin
				Red_wrdata_1a <= Filter_calc_4;
				Green_wrdata_1a <= Filter_calc_4;
				Blue_wrdata_1a <= Filter_calc_4;
				start_1_sec_counter <= 1'b0;
			end
			//CYCLE THROUGH ALL FILTERS
			3'd7 : begin
				start_1_sec_counter <= 1'b1;
			
				if (vsync_edge == 2'b10 && update_filter_flag) begin
					
					caseCounter<=caseCounter_buf;
					update_filter_flag<=1'b0;
				end 
				
				case(caseCounter)
						3'd0 : begin
				
							Red_wrdata_1a <= Red_rddata_0b;
							Green_wrdata_1a <= Green_rddata_0b;
							Blue_wrdata_1a <= Blue_rddata_0b;
							case7flag<=1'b0;
							if(~clock_1_s_buf && clock_1_s) begin

								caseCounter_buf<=3'd1;
								update_filter_flag<=1'b1;
							end
						end
						3'd1 : begin

							Red_wrdata_1a <= ~Red_rddata_0b;
							Green_wrdata_1a <= ~Green_rddata_0b;
							Blue_wrdata_1a <= ~Blue_rddata_0b;
							case7flag<=1'b0;
							if(~clock_1_s_buf && clock_1_s) begin

								caseCounter_buf<=3'd2;
								update_filter_flag<=1'b1;
							end
						end
						3'd2 : begin

							Red_wrdata_1a <= Y_calc;
							Green_wrdata_1a <= Y_calc;
							Blue_wrdata_1a <= Y_calc;
							case7flag<=1'b0;
							if(~clock_1_s_buf && clock_1_s) begin

								caseCounter_buf<=3'd3;
								update_filter_flag<=1'b1;
							end
						end
						3'd3 : begin

							Red_wrdata_1a <= ~Y_calc;
							Green_wrdata_1a <= ~Y_calc;
							Blue_wrdata_1a <= ~Y_calc;
							case7flag<=1'b0;
							if(~clock_1_s_buf && clock_1_s) begin

								caseCounter_buf<=3'd4;
								update_filter_flag<=1'b1;
							end
						end
						3'd4 : begin

							Red_wrdata_1a <= Filter_calc_2;
							Green_wrdata_1a <= Filter_calc_2;
							Blue_wrdata_1a <= Filter_calc_2;
							case7flag<=1'b0;
							if(~clock_1_s_buf && clock_1_s) begin

								caseCounter_buf<=3'd5;
								update_filter_flag<=1'b1;
							end
						end
						3'd5 : begin

							Red_wrdata_1a <= Filter_calc_3;
							Green_wrdata_1a <= Filter_calc_3;
							Blue_wrdata_1a <= Filter_calc_3;
							case7flag<=1'b0;
							if(~clock_1_s_buf && clock_1_s) begin

								caseCounter_buf<=3'd6;
								update_filter_flag<=1'b1;
								
							end
						end
						
						
						// FIVE TAP FILTER
						3'd6 : begin

							Red_wrdata_1a <= Filter_calc_4;
							Green_wrdata_1a <= Filter_calc_4;
							Blue_wrdata_1a <= Filter_calc_4;
							case7flag<=1'b1;
							if(~clock_1_s_buf && clock_1_s) begin
								caseCounter_buf<=3'd0;
								update_filter_flag<=1'b1;
							end
						end				
		
				endcase

			end
			
			
			
			
		endcase
	end
end

always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) DP_addr_1a <= 10'd0;
	else if (~Enable) DP_addr_1a <= 10'd0;
	else if (Clock_en & DP_wren_1a) 
		DP_addr_1a <= DP_addr_1a + 10'd1;
end

Filter_RAM Filter_RAM_R1 (
	.address_a(DP_addr_1a),
	.address_b(DP_addr_1b),
	.clock(Clock),
	.data_a(Red_wrdata_1a),
	.data_b(8'h00),
	.wren_a(DP_wren_1a),
	.wren_b(1'b0),
	.q_a(),
	.q_b(R_out)
);

Filter_RAM Filter_RAM_G1 (
	.address_a(DP_addr_1a),
	.address_b(DP_addr_1b),
	.clock(Clock),
	.data_a(Green_wrdata_1a),
	.data_b(8'h00),
	.wren_a(DP_wren_1a),
	.wren_b(1'b0),
	.q_a(),
	.q_b(G_out)
);

Filter_RAM Filter_RAM_B1 (
	.address_a(DP_addr_1a),
	.address_b(DP_addr_1b),
	.clock(Clock),
	.data_a(Blue_wrdata_1a),
	.data_b(8'h00),
	.wren_a(DP_wren_1a),
	.wren_b(1'b0),
	.q_a(),
	.q_b(B_out)
);
//999
logic [9:0] DP_addr_1b_reg;
assign DP_addr_1b = DP_addr_1b_reg + ((iRead_out_en) ? 10'd1 : 10'd0);

always_ff @(posedge Clock or negedge Resetn) begin
	if (~Resetn) DP_addr_1b_reg <= 10'h3FF;
	else if (~Enable) DP_addr_1b_reg <= 10'h3FF;
	else if (Clock_en)
		DP_addr_1b_reg <= DP_addr_1b;
end

endmodule
